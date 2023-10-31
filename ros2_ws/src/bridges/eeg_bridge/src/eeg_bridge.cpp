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
const double_t MAXIMUM_TIME_CORRECTION_ADJUSTMENT_PER_SYNC_TRIGGER = 0.001;

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

const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_TRIGGER_TOPIC = "/eeg/trigger";
const std::string HEALTHCHECK_TOPIC = "/eeg/healthcheck";

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

  this->init_socket();

  this->reset_session();

  this->eeg_bridge_state = WAITING_FOR_MTMS_DEVICE_START;
}

void EegBridge::create_publishers() {
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->eeg_sample_publisher = this->create_publisher<eeg_interfaces::msg::EegSample>(EEG_RAW_TOPIC, 10);
  this->trigger_publisher = this->create_publisher<eeg_interfaces::msg::Trigger>(EEG_TRIGGER_TOPIC, 10);
  this->eeg_info_publisher = this->create_publisher<eeg_interfaces::msg::EegInfo>(EEG_INFO_TOPIC, qos_persist_latest);
  this->publisher_healthcheck_ = this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);

  this->healthcheck_publisher_timer = this->create_wall_timer(std::chrono::milliseconds(500), std::bind(&EegBridge::publish_healthcheck, this));
}

void EegBridge::update_healthcheck(uint8_t status, std::string status_message, std::string actionable_message) {
  this->status = status;
  this->status_message = status_message;
  this->actionable_message = actionable_message;
}

void EegBridge::publish_healthcheck() {
  auto healthcheck = system_interfaces::msg::Healthcheck();

  healthcheck.status.value = status;
  healthcheck.status_message = status_message;
  healthcheck.actionable_message = actionable_message;

  this->publisher_healthcheck_->publish(healthcheck);
}

void EegBridge::subscribe_to_system_state() {
  this->session_been_stopped = false;
  this->system_state_received = false;

  auto system_state_callback = [this](const std::shared_ptr<mtms_device_interfaces::msg::SystemState> message) -> void {
    this->system_state_received = true;

    session_state = message->session_state;
    device_state = message->device_state;

    /* Stopping a session takes several seconds, whereas if another session is started immediately after the previous
       one is stopped, the mTMS device remains in "stopped" state only for a very short period of time. Hence, check both conditions
       to ensure that we notice if the session is stopped. */
    if (session_state.value == mtms_device_interfaces::msg::SessionState::STOPPING ||
        session_state.value == mtms_device_interfaces::msg::SessionState::STOPPED) {

      this->reset_session();
      this->session_been_stopped = true;
    }
  };

  /* HACK: Duplicates code from system_state_bridge.cpp. */
  const auto DEADLINE_NS = std::chrono::nanoseconds(SYSTEM_STATE_PUBLISHING_INTERVAL + SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE);

  auto qos = rclcpp::QoS(rclcpp::KeepLast(1))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
      .deadline(DEADLINE_NS)
      .lifespan(DEADLINE_NS);

  rclcpp::SubscriptionOptions subscription_options;
  subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "System state not received within deadline.");
  };

  this->system_state_subscriber = this->create_subscription<mtms_device_interfaces::msg::SystemState>("/mtms_device/system_state", qos,
                                                                                                        system_state_callback, subscription_options);
}

void EegBridge::reset_session() {
  first_trigger_received = false;
  time_correction = 0;
  sync_index = 1;
}

void EegBridge::wait_for_system_state() {
  RCLCPP_INFO(this->get_logger(), "Waiting for system state...");

  update_healthcheck(
    system_interfaces::msg::HealthcheckStatus::NOT_READY,
    "mTMS device not operational",
    "Please start the mTMS device.");

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
      } else {
        /* Timeout when reading EEG data indicates that the EEG measurement has stopped,
           change the state accordingly. */
        this->eeg_bridge_state = WAITING_FOR_MEASUREMENT_START;
      }

      uint8_t status_value;

      switch (this->eeg_bridge_state) {
        case WAITING_FOR_MTMS_DEVICE_START:
          this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::NOT_READY,
                                   "mTMS device is not operational",
                                   "Please start the mTMS device.");
          break;

        case WAITING_FOR_MEASUREMENT_START:
          this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::NOT_READY,
                                   "Waiting for EEG measurement to start",
                                   "Please start the measurement on the EEG device.");
          break;

        case WAITING_FOR_MEASUREMENT_STOP:
          this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::NOT_READY,
                                   "Waiting for EEG measurement to stop",
                                   "Please restart the measurement on the EEG device.");
          break;

        case WAITING_FOR_SESSION_START:
          this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::READY,
                                   "Ready",
                                   "Ready to start an experiment.");
          break;

        case WAITING_FOR_SESSION_STOP:
          this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::NOT_READY,
                                   "Waiting for session to stop",
                                   "Please stop the session on the mTMS device.");
          break;

        case STREAMING:
          /* Do not notify the UI if streaming and there is no error. */
          break;

        case ERROR_OUT_OF_SYNC:
          this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::ERROR,
                                   "Out of sync between EEG and mTMS device",
                                   "Please stop the session on the mTMS device.");
          break;
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
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 200000;
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
    RCLCPP_DEBUG(this->get_logger(), "No data received, reason: %s", strerror(errno));
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
  double_t new_time_correction = (sync_time - this->first_trigger_timestamp_) - this->sync_index * SYNC_INTERVAL;

  /* NOTE: Bittium's EEG device can be configured to send double triggers whenever an incoming trigger is received by the device. This happens when "Enabled"
     checkbox is ticked in Trigger A configuration in Settings menu. There seems to be no way to distinguish between the two triggers by the UDP packet contents,
     hence the additional check that we do not receive triggers too often. */
  if (abs(new_time_correction - this->time_correction) > MAXIMUM_TIME_CORRECTION_ADJUSTMENT_PER_SYNC_TRIGGER) {
    RCLCPP_ERROR(this->get_logger(), "Sync triggers received too frequently or infrequently. Check the BNC cable and EEG software configuration for double triggers.");

    this->eeg_bridge_state = ERROR_OUT_OF_SYNC;
  }

  this->time_correction = new_time_correction;
  this->sync_index++;
  RCLCPP_DEBUG(this->get_logger(), "Sync trigger received. Updated time correction to %f s.", this->time_correction);
}

void EegBridge::handle_trigger_packet() {
  double_t new_trigger_timestamp = read_time_from_buffer(TRIGGER_PACKET_FIRST_TIME_INDEX);

  uint8_t trigger_port = buffer[TRIGGER_PORT_INDEX] >> 4;

  /* Trigger port 1 corresponds to the sync trigger between the mTMS device and the EEG device. */
  if (trigger_port == 1) {
    if (!this->first_trigger_received) {

      /* Upon receiving the first trigger, reset time. */
      this->first_trigger_timestamp_ = new_trigger_timestamp;
      this->first_trigger_received = true;
      this->first_sample_of_session_ = true;

      RCLCPP_DEBUG(this->get_logger(), "'Session start' trigger received at time %.2f s.", this->first_trigger_timestamp_);
    } else {
      this->handle_sync_trigger(new_trigger_timestamp);
    }

  /* Trigger port 2 corresponds to the other trigger port between the mTMS device and the EEG device. */
  } else if (trigger_port == 2) {
    auto trigger_msg = eeg_interfaces::msg::Trigger();
    trigger_msg.time = new_trigger_timestamp - this->first_trigger_timestamp_ - this->time_correction;

    this->trigger_publisher->publish(trigger_msg);

    RCLCPP_INFO(this->get_logger(), "Published a trigger at time %.2f s.", trigger_msg.time);

  } else {
    RCLCPP_ERROR(this->get_logger(), "Unknown trigger port: %u", trigger_port);
  }
}

void EegBridge::handle_sample_packet() {
  uint16_t bundles = this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX] << 8 |
                     this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX + 1];

  if (bundles != 1) {
    RCLCPP_ERROR(this->get_logger(), "Invalid bundle size received: %u. The supported bundle size is 1. Please ensure that the sampling frequency and the packet frequency are set to the same value in the EEG software.", bundles);
    return;
  }

  double_t time = read_time_from_buffer(SAMPLE_PACKET_FIRST_TIME_INDEX);

  /* If sending trigger as channel, this will also initialize first_trigger_timestamp and first_trigger_received
     so also the next if statement will be executed. */
  if (this->send_trigger_as_channel && get_trigger_package_from_buffer() != 0) {
    this->publish_trigger_from_buffer(time);
    this->first_sample_of_session_ = true;
  }

  /* If first trigger has not been received yet, ignore the sample packet. */
  if (this->first_trigger_received) {

    if (time >= this->first_trigger_timestamp_) {
      double_t time_diff = time - this->first_trigger_timestamp_ - time_correction;

      this->publish_eeg_sample(time_diff);
      this->first_sample_of_session_ = false;

    } else {
      RCLCPP_WARN_THROTTLE(this->get_logger(),
                          *this->get_clock(),
                          1000,
                          "Sample packet arrived %.4f s before session start trigger. First trigger timestamp: %.4f, sample timestamp: %.4f.",
                          this->first_trigger_timestamp_ - time,
                          this->first_trigger_timestamp_,
                          time
      );
    }
  }
}

void EegBridge::handle_measurement_start_packet() {
  RCLCPP_DEBUG(this->get_logger(), "Measurement start packet received.");

  RCLCPP_INFO(this->get_logger(), "Measurement configuration:");

  this->measurement_start_packet_received_ = true;

  /* Parse measurement start packet. */

  this->sampling_frequency_ = (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX] << 24 |
                              (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 1] << 16 |
                              (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 2] << 8 |
                              (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 3];
  RCLCPP_INFO(this->get_logger(), "  - Sampling frequency set to %d Hz.", this->sampling_frequency_);

  this->num_of_channels_ = (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX] << 8 |
                           (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX + 1];

  this->send_trigger_as_channel = false;

  uint8_t num_of_eeg_channels = 0;
  uint8_t num_of_emg_channels = 0;
  for (uint8_t i = 0; i < this->num_of_channels_; i++) {
    uint16_t source_channel = buffer[MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX + 2 * i] << 8 |
                              buffer[MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX + 2 * i + 1];
    if (source_channel == SOURCE_CHANNEL_FOR_TRIGGER) {
      this->send_trigger_as_channel = true;
      continue;
    }

    /* The limits below come from NeurOne's amplifier configuration: each amplifier supports 40 channels in total; the monopolar (EEG)
       channels are numbered 1-32 and the bipolar (EMG) channels are numbered 33-40. The channels of the second amplifier are numbered
       after the channels of the first amplifier, thus, starting at 41. */
    if ((source_channel >= 33 && source_channel <= 40) || (source_channel >= 73 && source_channel <= 80)) {
      this->channel_types[i] = this->ChannelType::EMG;
      num_of_emg_channels++;
    } else {
      this->channel_types[i] = this->ChannelType::EEG;
      num_of_eeg_channels++;
    }
  }

  if (this->send_trigger_as_channel) {
    RCLCPP_INFO(this->get_logger(), "  - Trigger is sent as a channel.");

    this->num_of_channels_excluding_trigger_ = this->num_of_channels_ - 1;
  } else {
    RCLCPP_INFO(this->get_logger(), "  - Trigger is sent as a packet.");

    this->num_of_channels_excluding_trigger_ = this->num_of_channels_;
  }

  RCLCPP_INFO(this->get_logger(), "  - # of EEG channels: %d.", num_of_eeg_channels);
  RCLCPP_INFO(this->get_logger(), "  - # of EMG channels: %d.", num_of_emg_channels);

  RCLCPP_INFO(this->get_logger(), "  - Total # of EEG and EMG channels: %d.", this->num_of_channels_excluding_trigger_);

  /* Publish EEG info. */

  auto eeg_info_msg = eeg_interfaces::msg::EegInfo();

  eeg_info_msg.sampling_frequency = this->sampling_frequency_;
  eeg_info_msg.num_of_eeg_channels = num_of_eeg_channels;
  eeg_info_msg.num_of_emg_channels = num_of_emg_channels;
  eeg_info_msg.send_trigger_as_channel = this->send_trigger_as_channel;

  this->eeg_info_publisher->publish(eeg_info_msg);

  /* Reset number of sample packets received. */
  this->sample_packets_received_ = 0;
}

void EegBridge::handle_eeg_data_packet() {
  uint8_t packet_type = this->buffer[0];

  switch (packet_type) {
    case MEASUREMENT_START_PACKET_ID:
      this->handle_measurement_start_packet();
      break;

    case SAMPLE_PACKET_ID:
      this->sample_packets_received_ += 1;

      /* Ignore sample packets if in an error state, preventing streaming. */
      if (this->eeg_bridge_state == ERROR_OUT_OF_SYNC) {
        break;
      }

      if (!this->measurement_start_packet_received_) {
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "Streaming data on the EEG device but no measurement start packet received.");

        this->eeg_bridge_state = WAITING_FOR_MEASUREMENT_STOP;
        break;
      }

      if (!this->session_been_stopped) {
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "An mTMS session is ongoing, cannot synchronize EEG data with an ongoing session.");

        this->eeg_bridge_state = WAITING_FOR_SESSION_STOP;
        break;
      }

      if (this->session_state.value == mtms_device_interfaces::msg::SessionState::STARTED &&
          this->eeg_bridge_state == WAITING_FOR_SESSION_START) {

        this->eeg_bridge_state = STREAMING;
      }


      /* If session is started but sync triggers are not received, print a warning to check the connection between mTMS device and EEG device. */
      if (this->session_state.value == mtms_device_interfaces::msg::SessionState::STARTED &&
         !this->first_trigger_received) {

        RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Sync trigger not received. Please ensure: 1) 'Sync' port in the mTMS device is connected to 'Trigger A in' in the EEG device, and 2) sending triggers is enabled in the EEG software.");

        /* Wait for one second to receive the sync trigger before reporting an error. */
        if (this->sample_packets_received_ > this->sampling_frequency_) {
          this->eeg_bridge_state = ERROR_OUT_OF_SYNC;
        }
      }

      if (!this->send_trigger_as_channel) {

        /* If sending trigger as a packet, wait until we receive the first trigger and that the session is started. */
        if (this->first_trigger_received &&
            this->session_state.value == mtms_device_interfaces::msg::SessionState::STARTED) {

          this->handle_sample_packet();
        }

      } else {

        /* If sending trigger as channel, we need to handle packets before the first trigger is received as it will be
           sent as a part of a sample packet. When the first trigger is received, we also expect the session to be
           started. */
        if (!this->first_trigger_received ||
            this->session_state.value == mtms_device_interfaces::msg::SessionState::STARTED) {

          this->handle_sample_packet();
        }
      }

      if (this->device_state.value != mtms_device_interfaces::msg::DeviceState::OPERATIONAL) {
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "Waiting for mTMS device to start...");

        this->eeg_bridge_state = WAITING_FOR_MTMS_DEVICE_START;
        break;
      }

      if (this->session_state.value != mtms_device_interfaces::msg::SessionState::STARTED) {
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "Waiting for session to start...");

        this->eeg_bridge_state = WAITING_FOR_SESSION_START;
        break;
      }

      break;

    case TRIGGER_PACKET_ID:
      if (!this->session_been_stopped) {
        break;
      }

      if (this->measurement_start_packet_received_) {
        this->handle_trigger_packet();
      }
      break;

    case MEASUREMENT_END_PACKET_ID:
      RCLCPP_DEBUG(this->get_logger(), "Measurement end packet received.");
      RCLCPP_INFO(this->get_logger(), "Measurement ended on the EEG device.");

      this->session_been_stopped = false;
      break;

    default:
      RCLCPP_WARN(this->get_logger(), "Unknown packet type received: %u.", packet_type);
  }
}

int EegBridge::get_trigger_package_from_buffer() {
  auto index = FIRST_CHANNEL_INDEX + this->num_of_channels_excluding_trigger_ * 3;

  return (uint8_t) buffer[index] << 16 |
         (uint8_t) buffer[index + 1] << 8 |
         (uint8_t) buffer[index + 2];
}

void EegBridge::publish_trigger_from_buffer(double_t time) {
  int trigger_channel_package = get_trigger_package_from_buffer();

  /* TODO: Duplicates the logic in handle_trigger_packet function, deduplicate. */

  /* Trigger A is the sync trigger between the mTMS device and the EEG device. */
  if (trigger_channel_package == TRIGGER_A_IN) {
    if (!first_trigger_received) {
      this->first_trigger_timestamp_ = time;
      this->first_trigger_received = true;

      RCLCPP_DEBUG(this->get_logger(), "Session start trigger received, timestamp: %.4f", this->first_trigger_timestamp_);
    } else {
      this->handle_sync_trigger(time);
    }

  /* Trigger B is the other trigger between the mTMS device and the EEG device. */
  } else if (trigger_channel_package == TRIGGER_B_IN) {
    auto trigger_msg = eeg_interfaces::msg::Trigger();

    trigger_msg.time = time - this->first_trigger_timestamp_ - this->time_correction;
    this->trigger_publisher->publish(trigger_msg);

    RCLCPP_INFO(this->get_logger(), "Published a trigger at time %.2f s.", trigger_msg.time);

  } else {
    RCLCPP_ERROR(this->get_logger(), "Unknown trigger port");
  }
}

void EegBridge::publish_eeg_sample(double_t time_since_trigger) {

  auto message = eeg_interfaces::msg::EegSample();
  message.time = time_since_trigger;

  int i = FIRST_CHANNEL_INDEX;
  for (int channel = 0; channel < this->num_of_channels_excluding_trigger_; channel++) {

    int result = (uint8_t) buffer[i] << 16 |
                 (uint8_t) buffer[i + 1] << 8 |
                 (uint8_t) buffer[i + 2];

    if (result > SIGNED_MAX) {
      result -= UNSIGNED_MAX;
    }

    double result_uv = result;
    result_uv *= DC_MODE_SCALE;
    result_uv /= NANO_TO_MICRO_CONVERSION;

    if (channel_types[channel] == ChannelType::EEG) {
      message.eeg_data.push_back(result_uv);
    } else {
      message.emg_data.push_back(result_uv);
    }

    i += 3;
  }
  message.first_sample_of_session = this->first_sample_of_session_;

  this->eeg_sample_publisher->publish(message);

  RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Streaming EEG data into the topic: %s", EEG_RAW_TOPIC.c_str());
  RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Streaming EEG data...");
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_DEBUG(rclcpp::get_logger("eeg_bridge"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_DEBUG(rclcpp::get_logger("eeg_bridge"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); /* Allocate 10 MB. */
#endif

  node->spin();
  rclcpp::shutdown();
  return 0;
}
