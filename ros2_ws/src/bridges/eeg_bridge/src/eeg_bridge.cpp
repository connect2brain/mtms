#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <unistd.h>

#include "memory_utils.h"
#include "scheduling_utils.h"

#include "eeg_bridge.h"

#include "adapters/eeg_adapter.h"
#include "adapters/neurone_adapter.h"
#include "adapters/turbolink_adapter.h"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::placeholders;

/* Publisher topics */
const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string HEALTHCHECK_TOPIC = "/eeg/healthcheck";

/* Subscriber topics */
const std::string MTMS_DEVICE_HEALTHCHECK_TOPIC = "/mtms_device/healthcheck";
const std::string SYSTEM_SESSION_TOPIC = "/system/session";

/* Have a long queue to avoid dropping messages. */
const uint16_t EEG_QUEUE_LENGTH = 65535;

/* Note: Needs to match the values in session_bridge.cpp. */
const milliseconds SESSION_PUBLISHING_INTERVAL = 20ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 5ms;

const double_t SYNC_INTERVAL = 1.0;
const double_t MAXIMUM_TIME_CORRECTION_ADJUSTMENT_PER_SYNC_TRIGGER = 0.001;

EegBridge::EegBridge() : Node("eeg_bridge") {

  /* Port parameter */
  auto port_descriptor = rcl_interfaces::msg::ParameterDescriptor{};

  port_descriptor.description = "Port of the eeg device realtime output";
  port_descriptor.type = rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
  this->declare_parameter("port", NULL, port_descriptor);

  this->get_parameter("port", this->port);

  /* EEG device parameter */
  auto eeg_device_descriptor = rcl_interfaces::msg::ParameterDescriptor{};
  eeg_device_descriptor.description = "EEG device to use";
  eeg_device_descriptor.type = rcl_interfaces::msg::ParameterType::PARAMETER_STRING;
  this->declare_parameter("eeg-device", "neurone", eeg_device_descriptor);

  std::string eeg_device_type;
  this->get_parameter("eeg-device", eeg_device_type);

  /* Turbolink sampling frequency */
  auto turbolink_sampling_frequency_descriptor = rcl_interfaces::msg::ParameterDescriptor{};
  turbolink_sampling_frequency_descriptor.description = "Sampling frequency of a Turbolink device";
  turbolink_sampling_frequency_descriptor.type =
      rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
  this->declare_parameter("turbolink-sampling-frequency", 5000,
                          turbolink_sampling_frequency_descriptor);

  uint32_t turbolink_sampling_frequency;
  this->get_parameter("turbolink-sampling-frequency", turbolink_sampling_frequency);

  /* Turbolink channel count */
  auto turbolink_eeg_channel_count_descriptor = rcl_interfaces::msg::ParameterDescriptor{};
  turbolink_eeg_channel_count_descriptor.description = "EEG channel count of a Turbolink device";
  turbolink_eeg_channel_count_descriptor.type =
      rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
  this->declare_parameter("turbolink-eeg-channel-count", 64,
                          turbolink_eeg_channel_count_descriptor);

  uint8_t turbolink_eeg_channel_count;
  this->get_parameter("turbolink-eeg-channel-count", turbolink_eeg_channel_count);

  /* string to enum conversion should be done with cleaner solution at some
     point, maybe. */
  if (eeg_device_type == "neurone") {
    this->eeg_adapter = std::make_shared<NeurOneAdapter>(this->port);
  } else if (eeg_device_type == "turbolink") {
    this->eeg_adapter = std::make_shared<TurboLinkAdapter>(this->port, turbolink_sampling_frequency,
                                                           turbolink_eeg_channel_count);
  } else {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "Unsupported EEG device. %s",
                 eeg_device_type.c_str());
  }

  this->create_publishers();
  this->create_subscribers();
}

void EegBridge::create_publishers() {
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
                                .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
                                .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->eeg_sample_publisher =
      this->create_publisher<eeg_interfaces::msg::Sample>(EEG_RAW_TOPIC, EEG_QUEUE_LENGTH);

  this->eeg_info_publisher =
      this->create_publisher<eeg_interfaces::msg::EegInfo>(EEG_INFO_TOPIC, qos_persist_latest);
  this->healthcheck_publisher =
      this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);

  this->healthcheck_publisher_timer = this->create_wall_timer(
      std::chrono::milliseconds(500), [this] { publish_eeg_healthcheck(); });
}

void EegBridge::publish_eeg_healthcheck() {
  auto healtcheck = system_interfaces::msg::Healthcheck();

  healtcheck.status.value = this->status;
  healtcheck.status_message = this->status_message;
  healtcheck.actionable_message = this->actionable_message;

  this->healthcheck_publisher->publish(healtcheck);
}

void EegBridge::create_subscribers() {
  this->subscribe_to_session();
  this->subscribe_to_mtms_device_healthcheck();
}

void EegBridge::subscribe_to_session() {
  this->session_received = false;

  auto session_callback =
      [this](const std::shared_ptr<system_interfaces::msg::Session> message) -> void {
    this->session_received = true;

    this->session_state = message->state;

    /* Stopping a session takes several seconds, whereas if another session is
       started immediately after the previous one is stopped, the mTMS device
       remains in "stopped" state only for a very short period of time. Hence,
       check both conditions to ensure that we notice if the session is stopped.
     */
    if (session_state.value == system_interfaces::msg::SessionState::STOPPING ||
        session_state.value == system_interfaces::msg::SessionState::STOPPED) {
      RCLCPP_INFO(this->get_logger(), "Session stopped.");
      this->eeg_bridge_state = EegBridgeState::WAITING_FOR_SESSION_STOP;
      this->reset_session();
    }
  };

  /* HACK: Duplicates code from session_bridge.cpp. */
  const auto DEADLINE_NS =
      std::chrono::nanoseconds(SESSION_PUBLISHING_INTERVAL + SESSION_PUBLISHING_INTERVAL_TOLERANCE);

  auto qos = rclcpp::QoS(rclcpp::KeepLast(1))
                 .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
                 .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
                 .deadline(DEADLINE_NS)
                 .lifespan(DEADLINE_NS);

  rclcpp::SubscriptionOptions subscription_options;
  subscription_options.event_callbacks.deadline_callback =
      [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo &event) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                             "Session not received within deadline.");
      };

  this->session_subscriber = this->create_subscription<system_interfaces::msg::Session>(
      SYSTEM_SESSION_TOPIC, qos, session_callback, subscription_options);
}

void EegBridge::reset_session() {
  this->first_sample_of_session = true;

  this->first_sync_trigger_timestamp = UNSET_TIME;
  this->previous_sample_index = UNSET_PREVIOUS_SAMPLE_INDEX;

  this->time_correction = UNSET_TIME;
  this->time_offset = UNSET_TIME;
  this->num_of_sync_triggers_received = 0;

  /* Reset number of sample packets received. */
  this->sample_packets_received_since_session_start = 0;
}

void EegBridge::update_healthcheck(uint8_t status, std::string status_message,
                                   std::string actionable_message) {
  this->status = status;
  this->status_message = status_message;
  this->actionable_message = actionable_message;
}

void EegBridge::subscribe_to_mtms_device_healthcheck() {
  auto mtms_device_healtcheck_callback =
      [this](const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) -> void {
    this->mtms_device_available =
        (msg->status.value == system_interfaces::msg::HealthcheckStatus::READY);
  };

  this->mtms_device_healthcheck_subscriber =
      create_subscription<system_interfaces::msg::Healthcheck>(MTMS_DEVICE_HEALTHCHECK_TOPIC, 10,
                                                               mtms_device_healtcheck_callback);
}

void EegBridge::handle_sync_trigger(double_t sync_time) {
  this->num_of_sync_triggers_received++;

  if (first_sync_trigger_timestamp == UNSET_TIME) {
    first_sync_trigger_timestamp = sync_time;
  }

  double_t new_time_correction =
      (sync_time - first_sync_trigger_timestamp) - num_of_sync_triggers_received * SYNC_INTERVAL;

  if (abs(new_time_correction - this->time_correction) >
      MAXIMUM_TIME_CORRECTION_ADJUSTMENT_PER_SYNC_TRIGGER) {
    RCLCPP_ERROR(this->get_logger(),
                 "Sync triggers received too frequently or infrequently. Check the BNC cable and "
                 "EEG software configuration for double triggers.");
    this->eeg_bridge_state = ERROR_OUT_OF_SYNC;
  }

  this->time_correction = new_time_correction;
}

void EegBridge::handle_sample(eeg_interfaces::msg::Sample sample) {
  if (this->session_state.value != system_interfaces::msg::SessionState::STARTED) {
    RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                          "Waiting for session to start...");
    this->eeg_bridge_state = WAITING_FOR_SESSION_START;
    return;
  }

  this->sample_packets_received_since_session_start++;

  /* Ignore sample packets if in an error state, preventing streaming. */
  if (this->eeg_bridge_state == EegBridgeState::ERROR_OUT_OF_SYNC ||
      this->eeg_bridge_state == EegBridgeState::ERROR_SAMPLES_DROPPED) {
    return;
  }

  /* If mTMS device is available to send triggers AND first trigger has not been received yet,
     ignore the sample packet. */
  if (this->mtms_device_available && this->first_sync_trigger_timestamp == UNSET_TIME) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                         "Sync trigger not received. Please ensure: 1) 'Sync' port in the mTMS "
                         "device is connected to 'Trigger A in' in the EEG device, and 2) sending "
                         "triggers is enabled in the EEG software.");
    this->eeg_bridge_state = WAITING_FOR_SESSION_START;

    auto sampling_frequency = this->eeg_adapter->get_eeg_info().sampling_frequency;
    if (this->sample_packets_received_since_session_start > sampling_frequency) {
      this->eeg_bridge_state = ERROR_OUT_OF_SYNC;
    }
    return;
  }

  /* Check for dropped samples */
  if (previous_sample_index != UNSET_PREVIOUS_SAMPLE_INDEX &&
      sample.index != previous_sample_index + 1) {
    this->eeg_bridge_state = EegBridgeState::ERROR_SAMPLES_DROPPED;
    RCLCPP_ERROR(this->get_logger(), "Samples dropped.");
    return;
  }
  if (this->eeg_bridge_state == WAITING_FOR_SESSION_START) {
    RCLCPP_INFO(this->get_logger(), "Streaming data.");
  }
  this->eeg_bridge_state = EegBridgeState::STREAMING;

  this->previous_sample_index = sample.index;

  /* If mTMS device is not available offset samples by the timestamp of the first sample */
  if (this->first_sample_of_session && !this->mtms_device_available) {
    this->first_sample_of_session = false;
    this->time_offset = sample.time;
  }

  if (!this->mtms_device_available) {
    sample.time -= this->time_offset;
    this->eeg_sample_publisher->publish(sample);
    return;
  }

  if (this->first_sync_trigger_timestamp == UNSET_TIME ||
      sample.time < this->first_sync_trigger_timestamp) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                         "Sample packet arrived %.4f s before first sync trigger. First sync "
                         "trigger timestamp: %.4f, sample timestamp: %.4f.",
                         this->first_sync_trigger_timestamp - sample.time,
                         this->first_sync_trigger_timestamp, sample.time);
  }

  sample.time = sample.time - this->first_sync_trigger_timestamp - time_correction;
  eeg_sample_publisher->publish(sample);
}

void EegBridge::process_eeg_data_packet() {
  auto [result_type, sample, sync_time] = this->eeg_adapter->read_eeg_data_packet();

  auto eeg_info = this->eeg_adapter->get_eeg_info();

  if (eeg_info.sampling_frequency == UNSET_SAMPLING_FREQUENCY) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "No sampling frequency set");
  }

  switch (result_type) {
  case PacketResult::SAMPLE_WITH_SYNC:
    /* Handle sync before sample, as sync will update time_correction */
    handle_sync_trigger(sync_time);
    handle_sample(sample);
    break;
  case PacketResult::SAMPLE:
    handle_sample(sample);
    break;
  case PacketResult::SYNC_TRIGGER:
    handle_sync_trigger(sync_time);
    break;
  case PacketResult::INTERNAL:
    RCLCPP_DEBUG(this->get_logger(), "Internal adapter packet received.");
    break;
  case PacketResult::ERROR:
    RCLCPP_ERROR(this->get_logger(), "Error reading data packet.");
    this->eeg_bridge_state == EegBridgeState::WAITING_FOR_EEG_DEVICE;
    break;
  case PacketResult::END:
    RCLCPP_INFO(this->get_logger(), "EEG device measurement stopped.");
  default:
    RCLCPP_WARN(this->get_logger(), "Unknown result type while reading packet.");
  }
}

void EegBridge::wait_for_session() {
  RCLCPP_INFO(this->get_logger(), "Waiting for session...");

  auto base_interface = this->get_node_base_interface();

  /* HACK: Ensure that node stops itself gracefully by catching the exception:
     this is due to a known race condition in ROS2, in which if Ctrl-C (SIGINT)
     signal arrives between ok() and spin_some function calls, an exception is
     thrown. This seems to cause eProsima Fast DDS to occasionally go into a bad
     state, in which subscribers stop working properly after node is restarted.

     For more info about the race condition, see:

     https://github.com/ros2/rclcpp/issues/1066
     https://github.com/ros2/system_tests/pull/459
  */
  try {
    while (rclcpp::ok() && !this->session_received) {
      rclcpp::spin_some(base_interface);
    }
  } catch (const rclcpp::exceptions::RCLError &exception) {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "Failed with %s", exception.what());
  }
}

void EegBridge::spin() {
  /* Session has a deadline of 25 ms, but it will only start affecting once the first session
   is received. Hence, wait here until the session is received. */
  wait_for_session();

  RCLCPP_INFO(this->get_logger(), "Waiting for measurement start...");

  auto base_interface = this->get_node_base_interface();

  try {
    while (rclcpp::ok()) {
      rclcpp::spin_some(base_interface);

      process_eeg_data_packet();

      switch (this->eeg_bridge_state) {
      case EegBridgeState::WAITING_FOR_EEG_DEVICE:
        this->update_healthcheck(system_interfaces::msg::HealthcheckStatus ::NOT_READY,
                                 "Waiting for EEG measurement to start",
                                 "Please start the measurement on the EEG device.");
      case EegBridgeState::WAITING_FOR_SESSION_STOP:
        this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::NOT_READY,
                                 "Waiting for session to stop",
                                 "Please stop the session on the mTMS device.");
        break;
      case EegBridgeState::WAITING_FOR_SESSION_START:
        this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::READY, "Ready",
                                 "Ready");
        break;
      case EegBridgeState::STREAMING:
        this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::READY, "Streaming data",
                                 "Streaming data");
        break;
      case EegBridgeState::ERROR_OUT_OF_SYNC:
        this->update_healthcheck(
            system_interfaces::msg::HealthcheckStatus::ERROR,
            "Out of sync between EEG and mTMS device",
            /* XXX: Not really an actionable message at this stage; as long as we don't
            actually show the cause of the error in the UI, it is more useful to have the
            error as the 'actionable' message than not to display it at all. Once the causes
            come to the UI, this could be changed to "Please stop the session on the mTMS device"
            or similar. */
            "Out of sync between EEG and mTMS device.");
        break;
      case EegBridgeState::ERROR_SAMPLES_DROPPED:
        this->update_healthcheck(system_interfaces::msg::HealthcheckStatus::ERROR,
                                 "Samples dropped in EEG device", "Samples dropped in EEG device.");
        break;
      }
    }
  } catch (const rclcpp::exceptions::RCLError &exception) {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "Failed with %s", exception.what());
  }
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_DEBUG(rclcpp::get_logger("eeg_bridge"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY,
                        DEFAULT_REALTIME_SCHEDULING_PRIORITY);
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