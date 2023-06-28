#include "eeg_bridge.h"

#include "scheduling_utils.h"
#include "memory_utils.h"

#include <arpa/inet.h>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std::chrono;
using namespace std::chrono_literals;

const int32_t SIGNED_MAX = pow(2,23);
const int32_t UNSIGNED_MAX = pow(2,24);
const uint8_t DC_MODE_SCALE = 100;
const uint16_t NANO_TO_MICRO_CONVERSION = 1000;

const uint8_t MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX = 4;
const uint8_t MEASUREMENT_START_PACKET_N_CHANNELS_INDEX = 16;
const uint8_t MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX = 18;

const double_t SYNC_INTERVAL = 1.0;

/* HACK: If source channel matches the value below, it indicates the existence
 *  of trigger in the sample packet. This is documented in NeurOne's manual.
 *  However, it would be cleaner to have a separate field in measurement start
 *  packet to indicate the existence of trigger. */
const uint16_t SOURCE_CHANNEL_FOR_TRIGGER = 65535;

const uint8_t SAMPLE_PACKET_N_BUNDLES_INDEX = 10;

const uint8_t FIRST_CHANNEL_INDEX = 28;
const uint8_t SAMPLE_PACKET_FIRST_TIME_INDEX = 20;
const uint8_t TRIGGER_PACKET_FIRST_TIME_INDEX = 8;
const uint8_t TRIGGER_PORT_INDEX = 24;

const uint8_t MEASUREMENT_START_PACKET_ID = 1;
const uint8_t SAMPLE_PACKET_ID = 2;
const uint8_t TRIGGER_PACKET_ID = 3;
const uint8_t MEASUREMENT_END_PACKET_ID = 4;

const uint8_t TRIGGER_A_IN = 2;
const uint8_t TRIGGER_B_IN = 8;

const std::string EEG_RAW_TOPIC = "/eeg/raw_data";
const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_TRIGGER_TOPIC = "/eeg/trigger_received";

const uint8_t VERBOSE = 0;

/* HACK: Needs to match the values in system_state_bridge.cpp. */
const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL = 20ms;
const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE = 5ms;

EegBridge::EegBridge() : Node("eeg_bridge") {
  this->create_publishers();

  this->subscribe_to_system_state();

  /* Read ROS parameters. */

  auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};

  descriptor.description = "Port";
  descriptor.type = rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
  this->declare_parameter("port", NULL, descriptor);
  this->get_parameter("port", this->port_);

  /* HACK: Unfortunately, this is a terribly messy way of dividing the channels into EEG and EMG channels.
     Instead of trying to clean it up, we should ask for the manufacturer to provide more detailed
     information about individual channels in the measurement start packet. */

  descriptor.description = "EEG channel count for primary amplifier";
  this->declare_parameter("eeg_channels_primary_amplifier", NULL, descriptor);
  this->get_parameter("eeg_channels_primary_amplifier", this->eeg_channels_primary_amplifier_);

  descriptor.description = "EMG channel count for primary amplifier";
  this->declare_parameter("emg_channels_primary_amplifier", NULL, descriptor);
  this->get_parameter("emg_channels_primary_amplifier", this->emg_channels_primary_amplifier_);

  descriptor.description = "EEG channel count for secondary amplifier";
  this->declare_parameter("eeg_channels_secondary_amplifier", NULL, descriptor);
  this->get_parameter("eeg_channels_secondary_amplifier", this->eeg_channels_secondary_amplifier_);

  descriptor.description = "EMG channel count for secondary amplifier";
  this->declare_parameter("emg_channels_secondary_amplifier", NULL, descriptor);
  this->get_parameter("emg_channels_secondary_amplifier", this->emg_channels_secondary_amplifier_);

  this->set_channel_types();

  this->init_socket();

  this->reset_experiment();
}

void EegBridge::create_publishers() {
  const rmw_qos_profile_t qos_profile = {
      RMW_QOS_POLICY_HISTORY_KEEP_LAST,
      1,
      RMW_QOS_POLICY_RELIABILITY_RELIABLE,
      RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
      RMW_QOS_DEADLINE_DEFAULT,
      RMW_QOS_LIFESPAN_DEFAULT,
      RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT,
      RMW_QOS_LIVELINESS_LEASE_DURATION_DEFAULT,
      false
  };

  auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, qos_profile.depth), qos_profile);

  this->publisher_data_ = this->create_publisher<eeg_interfaces::msg::EegDatapoint>(EEG_RAW_TOPIC, 10);
  this->publisher_trigger_ = this->create_publisher<eeg_interfaces::msg::Trigger>(EEG_TRIGGER_TOPIC, qos);
  this->publisher_eeg_info_ = this->create_publisher<eeg_interfaces::msg::EegInfo>(EEG_INFO_TOPIC, qos);
}

void EegBridge::subscribe_to_system_state() {
  this->experiment_been_stopped = false;
  this->system_state_received = false;

  auto system_state_callback = [this](const std::shared_ptr<mtms_device_interfaces::msg::SystemState> message) -> void {
    this->system_state_received = true;
    experiment_state = message->experiment_state;

    /* Stopping an experiment takes several seconds, whereas if another experiment is started immediately after the previous
       one is stopped, the mTMS device remains in "stopped" state only for a very short period of time. Hence, check both conditions
       to ensure that we notice if the experiment is stopped. */
    if (experiment_state.value == mtms_device_interfaces::msg::ExperimentState::STOPPING ||
        experiment_state.value == mtms_device_interfaces::msg::ExperimentState::STOPPED) {

      this->reset_experiment();
      this->experiment_been_stopped = true;
    }
  };

  /* HACK: Duplicates code from system_state_bridge.cpp. */
  auto deadline = SYSTEM_STATE_PUBLISHING_INTERVAL + SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE;
  const uint64_t deadline_ns = static_cast<uint64_t>(std::chrono::nanoseconds(deadline).count());
  const rmw_time_t rmw_deadline = {0, deadline_ns};
  const rmw_time_t rmw_lifespan = rmw_deadline;

  const rmw_qos_profile_t qos_profile = {
      RMW_QOS_POLICY_HISTORY_KEEP_LAST,
      1,
      RMW_QOS_POLICY_RELIABILITY_RELIABLE,
      RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
      rmw_deadline,
      rmw_lifespan,
      RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT,
      RMW_QOS_LIVELINESS_LEASE_DURATION_DEFAULT,
      false
  };
  auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, qos_profile.depth), qos_profile);

  rclcpp::SubscriptionOptions subscription_options;
  subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) -> void {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "System state not received within deadline.");
  };

  this->subscription_system_state = this->create_subscription<mtms_device_interfaces::msg::SystemState>("/mtms_device/system_state", qos,
                                                                                                        system_state_callback, subscription_options);
}

void EegBridge::set_channel_types() {
  uint8_t channels_primary = this->eeg_channels_primary_amplifier_ + this->emg_channels_primary_amplifier_;
  uint8_t channels_secondary = this->eeg_channels_secondary_amplifier_ + this->emg_channels_secondary_amplifier_;

  uint8_t channels_total = channels_primary + channels_secondary;

  ChannelType type;
  for (uint8_t i = 1; i <= channels_total; i++) {
    if (i > channels_total - this->emg_channels_secondary_amplifier_) {
      type = this->ChannelType::EMG;
    } else if (i > this->eeg_channels_primary_amplifier_ && i <= channels_primary) {
      type = this->ChannelType::EMG;
    } else {
      type = this->ChannelType::EEG;
    }
    this->channel_types[i - 1] = type;
  }
}

void EegBridge::reset_experiment() {
  first_trigger_received = false;
  time_correction = 0;
  sync_index = 1;
}

void EegBridge::wait_for_system_state() {
  RCLCPP_INFO(this->get_logger(), "Waiting for system state...");

  auto base_interface = this->get_node_base_interface();

  /* HACK: Ensure that node stops itself gracefully by catching the exception:
     this is due to a known race condition in ROS2, in which if Ctrl-C (SIGINT) signal
     arrives between ok() and spin_some function calls, an exception is thrown. This
     seems to cause eProsima Fast DDS to occasionally go into a bad state, in which
     subscribers stop working properly after node is restarted.

     For more info about the race condition, see:

     https://github.com/ros2/rclcpp/issues/1066
     https://github.com/ros2/system_tests/pull/459
  */
  try {
    while (rclcpp::ok() && !this->system_state_received) {
      rclcpp::spin_some(base_interface);
    }
  } catch (const rclcpp::exceptions::RCLError & exception) {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "Failed with %s", exception.what());
  }
}

void EegBridge::spin() {
  /* System state has a deadline of 25 ms, but it will only start affecting once the first system state
     is received. Hence, wait here until the system state is received. */
  wait_for_system_state();

  RCLCPP_INFO(this->get_logger(), "Waiting for measurement start packet...");

  auto base_interface = this->get_node_base_interface();

  /* HACK: See comment in wait_for_system_state function. */
  try {
    while (rclcpp::ok()) {
      rclcpp::spin_some(base_interface);
      if (this->read_eeg_data_from_socket()) {
        this->handle_eeg_data_packet();
      }
    }
  } catch (const rclcpp::exceptions::RCLError & ex) {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "failed with %s", ex.what());
  }
}

void EegBridge::init_socket() {

  /* Init socket variable. */
  this->socket_length = sizeof(this->socket_other);

  /* Init socket file descriptor. */
  this->socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (this->socket_ == -1) {
    EegBridge::err("Failed to create socket file descriptor.");
  }

  /* Initialize socket_own with zeros. */
  memset((char *) &(this->socket_own), 0, sizeof(this->socket_own));

  socket_own.sin_family = AF_INET;
  socket_own.sin_port = htons(this->port_);
  socket_own.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(this->socket_, (struct sockaddr *) &(this->socket_own), sizeof(this->socket_own)) == -1) {
    EegBridge::err("Failed to bind socket file descriptor to socket.");
  }

  struct timeval read_timeout;
  read_timeout.tv_sec = 1;
  read_timeout.tv_usec = 0;
  setsockopt(this->socket_, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
}

void EegBridge::err(const char *message) {
  RCLCPP_ERROR(this->get_logger(), "Error: %s", message);
  exit(1);
}

bool EegBridge::read_eeg_data_from_socket() {
  auto success = recvfrom(this->socket_, this->buffer, BUFFER_LENGTH, 0, (struct sockaddr *) &(this->socket_other),
                          &(this->socket_length));

  if (success == -1) {
    RCLCPP_WARN(this->get_logger(), "No data received, reason: %s", strerror(errno));
    return false;
  }
  return true;
}

double_t EegBridge::read_time_from_buffer(uint8_t index) {
  uint64_t time_us = (uint64_t) buffer[index] << 56 |
                     (uint64_t) buffer[index + 1] << 48 |
                     (uint64_t) buffer[index + 2] << 40 |
                     (uint64_t) buffer[index + 3] << 32 |
                     (uint64_t) buffer[index + 4] << 24 |
                     (uint64_t) buffer[index + 5] << 16 |
                     (uint64_t) buffer[index + 6] << 8 |
                     (uint64_t) buffer[index + 7];
  return (double_t) time_us / 1000000.0;
}

void EegBridge::handle_sync_trigger(double_t sync_time) {
  time_correction = (sync_time - first_trigger_timestamp_) - sync_index * SYNC_INTERVAL;
  sync_index++;
  RCLCPP_INFO(this->get_logger(), "Sync trigger received. Updated time correction to %f s.", time_correction);
}

void EegBridge::handle_trigger_packet() {
  double_t new_trigger_timestamp = read_time_from_buffer(TRIGGER_PACKET_FIRST_TIME_INDEX);

  uint8_t trigger_index = buffer[TRIGGER_PORT_INDEX] >> 4;

  auto trigger_msg = eeg_interfaces::msg::Trigger();

  if (trigger_index == 1) {
    if (!this->first_trigger_received) {
      /* Upon receiving the first trigger, reset time. */
      this->first_trigger_timestamp_ = new_trigger_timestamp;
      this->first_trigger_received = true;
      this->first_sample_of_experiment_ = true;

      RCLCPP_INFO(this->get_logger(), "Experiment start trigger received, timestamp: %.2f s.", this->first_trigger_timestamp_);
    } else {
      this->handle_sync_trigger(new_trigger_timestamp);
    }
  } else {
    RCLCPP_INFO(this->get_logger(), "Trigger received in port: %u", trigger_index);
  }
  trigger_msg.time = new_trigger_timestamp - this->first_trigger_timestamp_ - this->time_correction;
  trigger_msg.index = trigger_index;

  this->publisher_trigger_->publish(trigger_msg);
}

void EegBridge::handle_sample_packet() {
  uint16_t bundles = this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX] << 8 |
                     this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX + 1];

  if (bundles != 1) {
    RCLCPP_WARN(this->get_logger(), "Warning: Bundle size %u not supported. Expected 1.", bundles);
    return;
  }

  double_t time = read_time_from_buffer(SAMPLE_PACKET_FIRST_TIME_INDEX);

  /* If sending trigger as channel, this will also initialize first_trigger_timestamp and first_trigger_received
     so also the next if statement will be executed. */
  if (this->send_trigger_as_channel && get_trigger_package_from_buffer() != 0) {
    this->publish_trigger_from_buffer(time);
    this->first_sample_of_experiment_ = true;
  }

  /* If first trigger has not been received yet, ignore the sample packet. */
  if (this->first_trigger_received) {

    if (time >= this->first_trigger_timestamp_) {
      double_t time_diff = time - this->first_trigger_timestamp_ - time_correction;

      this->publish_eeg_datapoint(time_diff);
      this->first_sample_of_experiment_ = false;

    } else {
      RCLCPP_WARN_THROTTLE(this->get_logger(),
                          *this->get_clock(),
                          1000,
                          "Sample packet arrived %.4f s before experiment start trigger. First trigger timestamp: %.4f, sample timestamp: %.4f.",
                          this->first_trigger_timestamp_ - time,
                          this->first_trigger_timestamp_,
                          time
      );
    }
  }
}

void EegBridge::handle_measurement_start_packet() {
  RCLCPP_INFO(this->get_logger(), "Measurement start packet received:");
  this->measurement_start_packet_received_ = true;

  /* Parse measurement start packet. */

  this->sampling_frequency_ = (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX] << 24 |
                              (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 1] << 16 |
                              (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 2] << 8 |
                              (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 3];
  RCLCPP_INFO(this->get_logger(), "  - Sampling frequency set to %d Hz.", this->sampling_frequency_);

  this->n_channels_ = (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX] << 8 |
                      (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX + 1];
  RCLCPP_INFO(this->get_logger(), "  - Number of channels set to %d.", this->n_channels_);

  this->send_trigger_as_channel = false;
  for (uint8_t i = 0; i < this->n_channels_; i++) {
    uint16_t source_channel = buffer[MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX + 2 * i] << 8 |
                              buffer[MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX + 2 * i + 1];
    if (source_channel == SOURCE_CHANNEL_FOR_TRIGGER) {
      this->send_trigger_as_channel = true;
    }
  }

  if (this->send_trigger_as_channel) {
    RCLCPP_INFO(this->get_logger(), "  - Trigger is sent as a channel.");

    this->n_channels_excluding_trigger_ = this->n_channels_ - 1;
  } else {
    RCLCPP_INFO(this->get_logger(), "  - Trigger is sent as a packet.");

    this->n_channels_excluding_trigger_ = this->n_channels_;
  }

  /* Publish EEG info. */

  auto eeg_info_msg = eeg_interfaces::msg::EegInfo();

  eeg_info_msg.sampling_frequency = this->sampling_frequency_;
  eeg_info_msg.n_channels = this->n_channels_;
  eeg_info_msg.send_trigger_as_channel = this->send_trigger_as_channel;

  this->publisher_eeg_info_->publish(eeg_info_msg);
}

void EegBridge::handle_eeg_data_packet() {
  uint8_t packet_type = this->buffer[0];

  switch (packet_type) {
    case MEASUREMENT_START_PACKET_ID:
      this->handle_measurement_start_packet();
      break;

    case SAMPLE_PACKET_ID:
      if (!this->measurement_start_packet_received_) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Streaming data on EEG device but no measurement start packet received. Please restart EEG streaming.");

        break;
      }

      if (!this->experiment_been_stopped) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Experiment is ongoing, cannot sync to an ongoing experiment. Please restart experiment.");

        break;
      }

      /* If experiment is started but sync triggers are not received, print a warning to check the connection between mTMS device and EEG device. */
      if (this->experiment_state.value == mtms_device_interfaces::msg::ExperimentState::STARTED &&
         !this->first_trigger_received) {

        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Sync trigger not received, please check the connection between mTMS device and EEG device.");
      }

      if (!this->send_trigger_as_channel) {

        /* If sending trigger as a packet, wait until we receive the first trigger and that the experiment is started. */
        if (this->first_trigger_received &&
            this->experiment_state.value == mtms_device_interfaces::msg::ExperimentState::STARTED) {

          this->handle_sample_packet();

        }

      } else {

        /* If sending trigger as channel, we need to handle packets before the first trigger is received as it will be
           sent as a part of a sample packet. When the first trigger is received, we also expect the experiment to be
           started. */
        if (!this->first_trigger_received ||
            this->experiment_state.value == mtms_device_interfaces::msg::ExperimentState::STARTED) {

          this->handle_sample_packet();
        }
      }

      if (this->experiment_state.value != mtms_device_interfaces::msg::ExperimentState::STARTED) {
        RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 60000, "Waiting for experiment to start...");
      }

      break;

    case TRIGGER_PACKET_ID:
      if (!this->experiment_been_stopped) {
        break;
      }

      if (this->measurement_start_packet_received_) {
        this->handle_trigger_packet();
      }
      break;

    case MEASUREMENT_END_PACKET_ID:
      RCLCPP_INFO(this->get_logger(), "Measurement end packet received.");

      break;

    default:
      RCLCPP_WARN(this->get_logger(), "Unknown packet type %u.", packet_type);
  }
}

int EegBridge::get_trigger_package_from_buffer() {
  auto index = FIRST_CHANNEL_INDEX + this->n_channels_excluding_trigger_ * 3;

  return (uint8_t) buffer[index] << 16 |
         (uint8_t) buffer[index + 1] << 8 |
         (uint8_t) buffer[index + 2];
}

void EegBridge::publish_trigger_from_buffer(double_t time) {
  int trigger_channel_package = get_trigger_package_from_buffer();

  auto trigger_msg = eeg_interfaces::msg::Trigger();

  if (trigger_channel_package == TRIGGER_A_IN) {
    trigger_msg.index = 1;
    trigger_msg.time = time - this->first_trigger_timestamp_;

    if (!first_trigger_received) {
      this->first_trigger_timestamp_ = time;
      this->first_trigger_received = true;

      RCLCPP_INFO(this->get_logger(), "Experiment start trigger received, timestamp: %.4f", this->first_trigger_timestamp_);
    } else {
      this->handle_sync_trigger(time);
    }

  } else if (trigger_channel_package == TRIGGER_B_IN) {
    trigger_msg.index = 2;
  }
  trigger_msg.time = time - this->first_trigger_timestamp_ - this->time_correction;

  this->publisher_trigger_->publish(trigger_msg);
}

void EegBridge::publish_eeg_datapoint(double_t time_since_trigger) {

  auto message = eeg_interfaces::msg::EegDatapoint();
  message.time = time_since_trigger;

  int i = FIRST_CHANNEL_INDEX;
  for (int channel = 1; channel <= this->n_channels_excluding_trigger_; channel++) {

    int result = (uint8_t) buffer[i] << 16 |
                 (uint8_t) buffer[i + 1] << 8 |
                 (uint8_t) buffer[i + 2];

    if (result > SIGNED_MAX) {
      result -= UNSIGNED_MAX;
    }

    double result_uv = result;
    result_uv *= DC_MODE_SCALE;
    result_uv /= NANO_TO_MICRO_CONVERSION;

    if (channel_types[channel - 1] == ChannelType::EEG) {
      message.eeg_channels.push_back(result_uv);
    } else {
      message.emg_channels.push_back(result_uv);
    }

    i += 3;
  }
  message.first_sample_of_experiment = this->first_sample_of_experiment_;

  this->publisher_data_->publish(message);
  RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 10000, "Streaming EEG data into topic %s.", EEG_RAW_TOPIC.c_str());
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); /* Allocate 10 MB. */
#endif

  node->spin();
  rclcpp::shutdown();
  return 0;
}
