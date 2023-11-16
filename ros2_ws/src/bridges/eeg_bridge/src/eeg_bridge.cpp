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
using namespace std::placeholders;

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

/* Have a long queue to avoid dropping messages. */
const uint16_t EEG_QUEUE_LENGTH = 65535;

const uint8_t VERBOSE = 0;

/* XXX: Needs to match the values in session_bridge.cpp. */
const milliseconds SESSION_PUBLISHING_INTERVAL = 20ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 5ms;

EegBridge::EegBridge() : Node("eeg_bridge") {
  this->create_publishers();

  this->subscribe_to_session();

  /* Subscriber for mTMS device healthcheck. */
  this->mtms_device_healthcheck_subscriber = create_subscription<system_interfaces::msg::Healthcheck>(
    "/mtms_device/healthcheck",
    10,
    std::bind(&EegBridge::handle_mtms_device_healthcheck, this, _1));

  /* Read ROS parameters. */

  auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};

  descriptor.description = "Port";
  descriptor.type = rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
  this->declare_parameter("port", NULL, descriptor);
  this->get_parameter("port", this->port_);

  this->init_socket();

  this->reset_session();

  this->eeg_bridge_state = WAITING_FOR_MEASUREMENT_START;
}

void EegBridge::create_publishers() {
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->eeg_sample_publisher = this->create_publisher<eeg_interfaces::msg::Sample>(
    EEG_RAW_TOPIC,
    EEG_QUEUE_LENGTH);

  this->trigger_publisher = this->create_publisher<eeg_interfaces::msg::Trigger>(EEG_TRIGGER_TOPIC, 10);
  this->eeg_info_publisher = this->create_publisher<eeg_interfaces::msg::EegInfo>(EEG_INFO_TOPIC, qos_persist_latest);
  this->healthcheck_publisher = this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);

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

  this->healthcheck_publisher->publish(healthcheck);
}

void EegBridge::handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) {
  this->mtms_device_available = msg->status.value == system_interfaces::msg::HealthcheckStatus::READY;
}

void EegBridge::subscribe_to_session() {
  this->session_been_stopped = false;
  this->session_received = false;

  auto session_callback = [this](const std::shared_ptr<system_interfaces::msg::Session> message) -> void {
    this->session_received = true;

    session_state = message->state;

    /* Stopping a session takes several seconds, whereas if another session is started immediately after the previous
       one is stopped, the mTMS device remains in "stopped" state only for a very short period of time. Hence, check both conditions
       to ensure that we notice if the session is stopped. */
    if (session_state.value == system_interfaces::msg::SessionState::STOPPING ||
        session_state.value == system_interfaces::msg::SessionState::STOPPED) {

      this->reset_session();
      this->session_been_stopped = true;
    }
  };

  /* HACK: Duplicates code from session_bridge.cpp. */
  const auto DEADLINE_NS = std::chrono::nanoseconds(SESSION_PUBLISHING_INTERVAL + SESSION_PUBLISHING_INTERVAL_TOLERANCE);

  auto qos = rclcpp::QoS(rclcpp::KeepLast(1))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
      .deadline(DEADLINE_NS)
      .lifespan(DEADLINE_NS);

  rclcpp::SubscriptionOptions subscription_options;
  subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Session not received within deadline.");
  };

  this->session_subscriber = this->create_subscription<system_interfaces::msg::Session>(
    "/system/session",
    qos,
    session_callback,
    subscription_options);
}

void EegBridge::reset_session() {
  first_sample_of_session = true;

  sync_trigger_received = false;
  time_correction = 0;
  num_of_sync_triggers_received = 0;
}

void EegBridge::wait_for_session() {
  RCLCPP_INFO(this->get_logger(), "Waiting for session...");

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
    while (rclcpp::ok() && !this->session_received) {
      rclcpp::spin_some(base_interface);
    }
  } catch (const rclcpp::exceptions::RCLError & exception) {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "Failed with %s", exception.what());
  }
}

void EegBridge::spin() {
  /* Session has a deadline of 25 ms, but it will only start affecting once the first session
     is received. Hence, wait here until the session is received. */
  wait_for_session();

  RCLCPP_INFO(this->get_logger(), "Waiting for measurement start packet...");

  auto base_interface = this->get_node_base_interface();

  /* HACK: See comment in wait_for_session function. */
  try {
    while (rclcpp::ok()) {
      rclcpp::spin_some(base_interface);
      if (this->read_eeg_data_from_socket()) {
        auto start_time = std::chrono::high_resolution_clock::now();

        this->handle_eeg_data_packet();

        /* Measure the processing time for the packet. */
        auto end_time = std::chrono::high_resolution_clock::now();
        double_t processing_time = std::chrono::duration<double_t>(end_time - start_time).count();

        RCLCPP_DEBUG(this->get_logger(), "Processing time for packet: %.6f s.", processing_time);
      } else {
        RCLCPP_DEBUG(this->get_logger(), "Timeout while reading EEG data");

        /* Timeout while reading EEG data indicates that the EEG measurement has stopped,
           change the state accordingly. */
        this->eeg_bridge_state = WAITING_FOR_MEASUREMENT_START;
      }

      switch (this->eeg_bridge_state) {
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
                                   "Ready");
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
                                   /* XXX: Not really an actionable message at this stage; as long as we don't
                                   actually show the cause of the error in the UI, it is more useful to have the
                                   error as the 'actionable' message than not to display it at all. Once the causes
                                   come to the UI, this could be changed to "Please stop the session on the mTMS device"
                                   or similar. */
                                   "Out of sync between EEG and mTMS device.");
          break;

        case ERROR_SAMPLES_DROPPED:
          this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::ERROR,
                                   "Samples dropped in EEG device",
                                   "Samples dropped in EEG device.");
          break;
      }
    }
  } catch (const rclcpp::exceptions::RCLError & ex) {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "failed with %s", ex.what());
  }
}

void EegBridge::init_socket() {
  RCLCPP_INFO(this->get_logger(), "Binding to port: %d", this->port_);

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
  num_of_sync_triggers_received++;
  double_t new_time_correction = (sync_time - first_sync_trigger_timestamp) - num_of_sync_triggers_received * SYNC_INTERVAL;

  /* NOTE: Bittium's EEG device can be configured to send double triggers whenever an incoming trigger is received by the device. This happens when "Enabled"
     checkbox is ticked in Trigger A configuration in Settings menu. There seems to be no way to distinguish between the two triggers by the UDP packet contents,
     hence the additional check that we do not receive triggers too often. */
  if (abs(new_time_correction - this->time_correction) > MAXIMUM_TIME_CORRECTION_ADJUSTMENT_PER_SYNC_TRIGGER) {
    RCLCPP_ERROR(this->get_logger(), "Sync triggers received too frequently or infrequently. Check the BNC cable and EEG software configuration for double triggers.");

    this->eeg_bridge_state = ERROR_OUT_OF_SYNC;
  }

  this->time_correction = new_time_correction;
  RCLCPP_DEBUG(this->get_logger(), "Sync trigger received. Updated time correction to %f s.", this->time_correction);
}

void EegBridge::handle_trigger_packet() {
  double_t trigger_timestamp = read_time_from_buffer(TRIGGER_PACKET_FIRST_TIME_INDEX);

  uint8_t trigger_port = buffer[TRIGGER_PORT_INDEX] >> 4;

  /* Trigger port 1 corresponds to the sync trigger between the mTMS device and the EEG device. */
  if (trigger_port == 1) {
    if (!this->sync_trigger_received) {

      /* Upon receiving the first sync trigger, reset time. */
      this->first_sync_trigger_timestamp = trigger_timestamp;
      this->sync_trigger_received = true;

      RCLCPP_DEBUG(this->get_logger(), "'Session start' trigger received at time %.2f s.", this->first_sync_trigger_timestamp);
    } else {
      this->handle_sync_trigger(trigger_timestamp);
    }

  /* Trigger port 2 corresponds to the other trigger port between the mTMS device and the EEG device. */
  } else if (trigger_port == 2) {
    auto trigger_msg = eeg_interfaces::msg::Trigger();
    trigger_msg.time = trigger_timestamp - this->first_sync_trigger_timestamp - this->time_correction;

    this->trigger_publisher->publish(trigger_msg);

    RCLCPP_INFO(this->get_logger(), "Published a trigger at time %.2f s.", trigger_msg.time);

  } else {
    RCLCPP_ERROR(this->get_logger(), "Unknown trigger port: %u", trigger_port);
  }
}

/* XXX: Very close to a similar check in eeg_gatherer.cpp and other pipeline stages. Unify? */
void EegBridge::check_dropped_samples(double_t sample_time) {
  if (this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {
    RCLCPP_WARN(this->get_logger(), "Sampling frequency not received, cannot check for dropped samples.");
  }

  if (this->sampling_frequency != UNSET_SAMPLING_FREQUENCY &&
      this->previous_time) {

    auto time_diff = sample_time - this->previous_time;
    auto threshold = this->sampling_period + this->TOLERANCE_S;

    if (time_diff > threshold) {
      /* Err if sample(s) were dropped. */
      this->eeg_bridge_state = EegBridgeState::ERROR_SAMPLES_DROPPED;

      RCLCPP_ERROR(this->get_logger(),
          "Sample(s) dropped. Time difference between consecutive samples: %.5f, should be: %.5f, limit: %.5f", time_diff, this->sampling_period, threshold);

    } else {
      /* If log-level is set to DEBUG, print time difference for all samples, regardless of if samples were dropped or not. */
      RCLCPP_DEBUG(this->get_logger(),
        "Time difference between consecutive samples: %.5f", time_diff);
    }
  }
  this->previous_time = sample_time;
}

void EegBridge::handle_sample_packet() {
  uint16_t bundles = this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX] << 8 |
                     this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX + 1];

  if (bundles != 1) {
    RCLCPP_ERROR(this->get_logger(), "Invalid bundle size received: %u. The supported bundle size is 1. Please ensure that the sampling frequency and the packet frequency are set to the same value in the EEG software.", bundles);
    return;
  }

  double_t time = read_time_from_buffer(SAMPLE_PACKET_FIRST_TIME_INDEX);

  check_dropped_samples(time);

  /* If sending trigger as channel, this will also initialize first_trigger_timestamp and sync_trigger_received
     so also the next if statement will be executed. */
  if (this->send_trigger_as_channel && get_trigger_package_from_buffer() != 0) {
    this->publish_trigger_from_buffer(time);
  }

  /* If mTMS device is available to send triggers AND first trigger has not been received yet, ignore the sample packet. */
  if (!this->sync_trigger_received && this->mtms_device_available) {
    return;
  }

  if (this->mtms_device_available) {
    /* If mTMS device is available, publish samples with corrected time. */
    if (time >= this->first_sync_trigger_timestamp) {
      double_t corrected_time = time - this->first_sync_trigger_timestamp - time_correction;

      this->publish_eeg_sample(corrected_time);
      this->first_sample_of_session = false;

    } else {
      RCLCPP_WARN_THROTTLE(this->get_logger(),
                          *this->get_clock(),
                          1000,
                          "Sample packet arrived %.4f s before first sync trigger. First sync trigger timestamp: %.4f, sample timestamp: %.4f.",
                          this->first_sync_trigger_timestamp - time,
                          this->first_sync_trigger_timestamp,
                          time
      );
    }
  } else {
    /* If mTMS device is not available, adjust all sample timestamps based on the timestamp of the first sample
       in the session, so that the first sample in the session will get the timestamp 0.0, etc. */
    if (this->first_sample_of_session) {

      /* XXX: It is slightly incorrect to use 'time_correction' variable for this, as if the mTMS device is available, the
        same variable is used for sync-trigger-based time correction, which is semantically different from this use. */
      this->time_correction = time;
      this->first_sample_of_session = false;
    }
    double_t corrected_time = time - this->time_correction;
    this->publish_eeg_sample(corrected_time);
  }
}

void EegBridge::handle_measurement_start_packet() {
  RCLCPP_DEBUG(this->get_logger(), "Measurement start packet received.");

  RCLCPP_INFO(this->get_logger(), "Measurement configuration:");

  this->measurement_start_packet_received_ = true;

  /* Parse measurement start packet. */

  this->sampling_frequency = (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX] << 24 |
                             (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 1] << 16 |
                             (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 2] << 8 |
                             (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 3];
  RCLCPP_INFO(this->get_logger(), "  - Sampling frequency set to %d Hz.", this->sampling_frequency);

  this->sampling_period = 1.0 / this->sampling_frequency;

  this->num_of_channels_ = (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX] << 8 |
                           (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX + 1];

  this->send_trigger_as_channel = false;

  this->num_of_eeg_channels = 0;
  this->num_of_emg_channels = 0;
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
      this->num_of_emg_channels++;
    } else {
      this->channel_types[i] = this->ChannelType::EEG;
      this->num_of_eeg_channels++;
    }
  }

  if (this->send_trigger_as_channel) {
    RCLCPP_INFO(this->get_logger(), "  - Trigger is sent as a channel.");

    this->num_of_channels_excluding_trigger_ = this->num_of_channels_ - 1;
  } else {
    RCLCPP_INFO(this->get_logger(), "  - Trigger is sent as a packet.");

    this->num_of_channels_excluding_trigger_ = this->num_of_channels_;
  }

  RCLCPP_INFO(this->get_logger(), "  - # of EEG channels: %d.", this->num_of_eeg_channels);
  RCLCPP_INFO(this->get_logger(), "  - # of EMG channels: %d.", this->num_of_emg_channels);

  RCLCPP_INFO(this->get_logger(), "  - Total # of EEG and EMG channels: %d.", this->num_of_channels_excluding_trigger_);

  /* Publish EEG info. */

  auto eeg_info_msg = eeg_interfaces::msg::EegInfo();

  eeg_info_msg.sampling_frequency = this->sampling_frequency;
  eeg_info_msg.num_of_eeg_channels = this->num_of_eeg_channels;
  eeg_info_msg.num_of_emg_channels = this->num_of_emg_channels;
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
      if (this->eeg_bridge_state == ERROR_OUT_OF_SYNC ||
          this->eeg_bridge_state == ERROR_SAMPLES_DROPPED) {
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

      if (this->session_state.value == system_interfaces::msg::SessionState::STARTED &&
          this->eeg_bridge_state == WAITING_FOR_SESSION_START) {

        this->eeg_bridge_state = STREAMING;
      }

      /* If session is started AND mTMS device is available, but sync triggers are not received, print a warning to check the
         connection between mTMS device and EEG device. */
      if (this->mtms_device_available &&
          this->session_state.value == system_interfaces::msg::SessionState::STARTED &&
          !this->sync_trigger_received) {

        RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Sync trigger not received. Please ensure: 1) 'Sync' port in the mTMS device is connected to 'Trigger A in' in the EEG device, and 2) sending triggers is enabled in the EEG software.");

        /* Wait for one second to receive the sync trigger before reporting an error. */
        if (this->sample_packets_received_ > this->sampling_frequency) {
          this->eeg_bridge_state = ERROR_OUT_OF_SYNC;
        }
      }

      if (this->session_state.value != system_interfaces::msg::SessionState::STARTED) {
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "Waiting for session to start...");

        this->eeg_bridge_state = WAITING_FOR_SESSION_START;
        break;
      }

      /* If mTMS device is not available, always handle sample packets; do not worry about sync triggers. */
      if (!this->mtms_device_available) {
        this->handle_sample_packet();
        break;
      }

      /* If sending trigger as a packet, wait until we receive the first trigger and that the session is started. */
      if (!this->send_trigger_as_channel) {

        if (this->sync_trigger_received &&
            this->session_state.value == system_interfaces::msg::SessionState::STARTED) {

          this->handle_sample_packet();
        }

      } else {
        /* If sending trigger as channel, we need to handle packets before the first trigger is received as it will be
           sent as a part of a sample packet. When the first trigger is received, we also expect the session to be
           started. */

        /* XXX: It seems that this logic could be cleaned up. */
        if (!this->sync_trigger_received ||
            this->session_state.value == system_interfaces::msg::SessionState::STARTED) {

          this->handle_sample_packet();
        }
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
    if (!sync_trigger_received) {
      this->first_sync_trigger_timestamp = time;
      this->sync_trigger_received = true;

      RCLCPP_DEBUG(this->get_logger(), "Session start trigger received, timestamp: %.4f", this->first_sync_trigger_timestamp);
    } else {
      this->handle_sync_trigger(time);
    }

  /* Trigger B is the other trigger between the mTMS device and the EEG device. */
  } else if (trigger_channel_package == TRIGGER_B_IN) {
    auto trigger_msg = eeg_interfaces::msg::Trigger();

    trigger_msg.time = time - this->first_sync_trigger_timestamp - this->time_correction;
    this->trigger_publisher->publish(trigger_msg);

    RCLCPP_INFO(this->get_logger(), "Published a trigger at time %.2f s.", trigger_msg.time);

  } else {
    RCLCPP_ERROR(this->get_logger(), "Unknown trigger port");
  }
}

void EegBridge::publish_eeg_sample(double_t time) {

  auto message = eeg_interfaces::msg::Sample();
  message.time = time;

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
  message.metadata.sampling_frequency = this->sampling_frequency;
  message.metadata.num_of_eeg_channels = this->num_of_eeg_channels;
  message.metadata.num_of_emg_channels = this->num_of_emg_channels;
  message.metadata.first_sample_of_session = this->first_sample_of_session;

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
