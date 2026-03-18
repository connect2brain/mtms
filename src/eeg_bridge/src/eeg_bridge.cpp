#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <unistd.h>

#include "realtime_utils/utils.h"

#include "eeg_bridge.h"

#include "adapters/eeg_adapter.h"
#include "adapters/neurone_adapter.h"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::placeholders;

/* Publisher topics */
const std::string EEG_RAW_TOPIC = "/mtms/eeg/raw";
const std::string DEVICE_INFO_TOPIC = "/mtms/eeg_device/info";
const std::string DROPPED_SAMPLES_TOPIC = "/eeg_bridge/dropped_samples";

/* Have a long queue to avoid dropping messages. */
const uint16_t EEG_QUEUE_LENGTH = 65535;

/* Sample timeout in milliseconds */
const int64_t SAMPLE_TIMEOUT_MS = 200;

EegBridge::EegBridge() : Node("eeg_bridge") {
  RCLCPP_INFO(this->get_logger(), "Initializing EEG bridge...");

  /* Create publishers */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
                                .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
                                .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->eeg_sample_publisher =
      this->create_publisher<eeg_interfaces::msg::Sample>(EEG_RAW_TOPIC, EEG_QUEUE_LENGTH);

  this->device_info_publisher =
      this->create_publisher<eeg_interfaces::msg::EegDeviceInfo>(DEVICE_INFO_TOPIC, qos_persist_latest);

  this->dropped_samples_publisher =
    this->create_publisher<std_msgs::msg::Int32>(DROPPED_SAMPLES_TOPIC, 10);

  /* Create subscribers */
  this->global_config_subscription = this->create_subscription<system_interfaces::msg::GlobalConfig>(
      "/mtms/global_configurator/config",
      qos_persist_latest,
      std::bind(&EegBridge::handle_global_config, this, std::placeholders::_1));

}

void EegBridge::handle_global_config(const system_interfaces::msg::GlobalConfig::SharedPtr msg) {
  bool needs_adapter_recreation = false;
  bool needs_socket_recreation = false;

  /* Check if port has changed */
  if (msg->eeg_port != static_cast<int32_t>(this->port)) {
    this->port = static_cast<uint16_t>(msg->eeg_port);
    needs_socket_recreation = true;
    needs_adapter_recreation = true;
  }

  /* Check if device type has changed */
  if (msg->eeg_device != this->eeg_device_type) {
    this->eeg_device_type = msg->eeg_device;
    needs_adapter_recreation = true;
  }

  /* Update maximum dropped samples */
  if (msg->maximum_dropped_samples != static_cast<int32_t>(this->maximum_dropped_samples)) {
    this->maximum_dropped_samples = static_cast<uint8_t>(msg->maximum_dropped_samples);
  }

  /* Recreate socket if port changed */
  if (needs_socket_recreation) {
    this->socket_ = std::make_shared<UdpSocket>(this->port);
    if (!this->socket_->init_socket()) {
      RCLCPP_ERROR(this->get_logger(), "Failed to initialize UDP socket");
      return;
    }
  }

  /* Recreate adapter if needed */
  if (needs_adapter_recreation) {
    /* Ensure socket exists before creating adapter */
    if (!this->socket_) {
      this->socket_ = std::make_shared<UdpSocket>(this->port);
      if (!this->socket_->init_socket()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize UDP socket");
        return;
      }
    }

    if (this->eeg_device_type == "neurone") {
      this->eeg_adapter = std::make_shared<NeurOneAdapter>(this->socket_);
    } else {
      this->eeg_adapter.reset();
      RCLCPP_ERROR(this->get_logger(), "Unsupported EEG device: %s", this->eeg_device_type.c_str());
      return;
    }

    /* Publish updated device info */
    publish_device_info();
  }

  /* Log final configuration */
  RCLCPP_INFO(this->get_logger(), "EEG bridge configuration:");
  RCLCPP_INFO(this->get_logger(), "  • Port: %d", this->port);
  RCLCPP_INFO(this->get_logger(), "  • EEG device: %s", this->eeg_device_type.c_str());
  RCLCPP_INFO(this->get_logger(), "  • Maximum dropped samples: %d", this->maximum_dropped_samples);
  RCLCPP_INFO(this->get_logger(), " ");

}

void EegBridge::publish_device_info() {
  if (!this->eeg_adapter) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                         "Cannot publish device info: EEG adapter not initialized");
    return;
  }
  auto device_info = this->eeg_adapter->get_device_info();
  device_info.is_streaming = (this->device_state == EegDeviceState::EEG_DEVICE_STREAMING);
  this->device_info_publisher->publish(device_info);
}

void EegBridge::set_device_state(EegDeviceState new_state) {
  EegDeviceState previous_state = this->device_state;
  this->device_state = new_state;

  if (previous_state != this->device_state) {
    if (previous_state != EegDeviceState::EEG_DEVICE_STREAMING &&
        this->device_state == EegDeviceState::EEG_DEVICE_STREAMING) {
      reset_state();
    }
    publish_device_info();
  }
}

void EegBridge::publish_cumulative_dropped_samples() {
  auto dropped_samples_msg = std_msgs::msg::Int32();
  dropped_samples_msg.data = static_cast<int32_t>(this->cumulative_dropped_samples);
  this->dropped_samples_publisher->publish(dropped_samples_msg);
}

bool EegBridge::reset_state() {
  this->session_sample_index = 0;
  this->time_offset = UNSET_TIME;
  this->previous_device_sample_index = UNSET_PREVIOUS_SAMPLE_INDEX;
  this->cumulative_dropped_samples = 0;
  this->last_sample_time = std::chrono::steady_clock::now();

  this->publish_cumulative_dropped_samples();

  return true;
}

bool EegBridge::check_for_dropped_samples(uint64_t device_sample_index) {
  /* Warn if the device sample index wraps around. */
  if (previous_device_sample_index != UNSET_PREVIOUS_SAMPLE_INDEX &&
      device_sample_index == 0 &&
      previous_device_sample_index > 0) {

    RCLCPP_WARN(this->get_logger(), 
                "Device sample index wrapped around. Previous: %lu, current: %lu.",
                previous_device_sample_index,
                device_sample_index);
  }

  /* Track any dropped samples using device indices. */
  if (previous_device_sample_index != UNSET_PREVIOUS_SAMPLE_INDEX &&
      device_sample_index > previous_device_sample_index + 1 &&
      /* Ignore the case where the sample index wraps around. */
      device_sample_index != 0) {

    const auto dropped_samples_count = static_cast<uint64_t>(device_sample_index - previous_device_sample_index - 1);

    this->cumulative_dropped_samples += dropped_samples_count;
    this->publish_cumulative_dropped_samples();

    RCLCPP_WARN(this->get_logger(),
      "Samples dropped (%lu, cumulative %lu). Previous device sample index: %lu, current: %lu.",
      dropped_samples_count,
      this->cumulative_dropped_samples,
      previous_device_sample_index,
      device_sample_index);

    if (dropped_samples_count > this->maximum_dropped_samples) {
      RCLCPP_ERROR(this->get_logger(), "Samples dropped above allowed threshold (%d).", this->maximum_dropped_samples);
    }
  }
  
  this->previous_device_sample_index = device_sample_index;
  return true;  // No samples dropped
}

void EegBridge::check_for_sample_timeout() {
  if (this->device_state != EegDeviceState::EEG_DEVICE_STREAMING) {
    return;
  }

  auto now = std::chrono::steady_clock::now();
  auto time_since_last_sample = std::chrono::duration_cast<std::chrono::milliseconds>(
    now - this->last_sample_time).count();

  if (time_since_last_sample > SAMPLE_TIMEOUT_MS) {
    RCLCPP_ERROR(this->get_logger(),
                 "Sample timeout: No sample received in %ld ms",
                 time_since_last_sample);
    set_device_state(EegDeviceState::WAITING_FOR_EEG_DEVICE);
  }
}

eeg_interfaces::msg::Sample EegBridge::create_ros_sample(const AdapterSample& adapter_sample,
                                                    const eeg_interfaces::msg::EegDeviceInfo& [[maybe_unused]] device_info) {
  auto sample = eeg_interfaces::msg::Sample();
  sample.eeg = adapter_sample.eeg;
  sample.emg = adapter_sample.emg;
  sample.time = adapter_sample.time;
  sample.pulse_trigger = adapter_sample.trigger_a;
  sample.loopback_trigger = adapter_sample.trigger_b;
  
  return sample;
}

void EegBridge::handle_sample(eeg_interfaces::msg::Sample sample) {
  /* If this is the first sample, set the time offset. */
  if (std::isnan(this->time_offset)) {
    this->time_offset = sample.time;
  }

  sample.time -= this->time_offset;

  sample.is_session_start = false;

  /* Set the streaming sample index. */
  sample.sample_index = this->session_sample_index;
  this->session_sample_index++;

  /* Mark the sample as valid by default. The preprocessor can later mark it as invalid if needed. */
  sample.valid = true;

  this->eeg_sample_publisher->publish(sample);

  // Log latency trigger when present
  if (sample.loopback_trigger) {
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Receiving loopback triggers");
  }
  
  // Log pulse trigger when present
  if (sample.pulse_trigger) {
    RCLCPP_INFO(this->get_logger(), "Received TMS pulse at time: %.4f s", sample.time);
  }

}

void EegBridge::process_eeg_packet() {
  /* Check if adapter is initialized */
  if (!this->eeg_adapter) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                         "Waiting for global configuration to initialize EEG adapter...");
    return;
  }

  /* Check if socket is initialized */
  if (!this->socket_) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                         "Waiting for socket initialization...");
    return;
  }

  // Read UDP packet
  if (!this->socket_->read_data(this->buffer, BUFFER_SIZE)) {
    RCLCPP_WARN(this->get_logger(), "Timeout while reading EEG data");

    // Interpret timeout as the end of the stream, e.g. if the device has
    // stopped sending data or if the ethernet cable is disconnected.
    set_device_state(EegDeviceState::WAITING_FOR_EEG_DEVICE);

    return;
  }

  // Process the packet
  AdapterPacket packet = this->eeg_adapter->process_packet(this->buffer, BUFFER_SIZE);

  auto device_info = this->eeg_adapter->get_device_info();

  if (device_info.sampling_frequency == UNSET_SAMPLING_FREQUENCY) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "No sampling frequency set");
    return;
  }

  switch (packet.result) {

  case SAMPLE: {
    set_device_state(EegDeviceState::EEG_DEVICE_STREAMING);

    /* Track device activity while stream packets are flowing. */
    this->last_sample_time = std::chrono::steady_clock::now();

    // Check for dropped samples using device sample index
    if (!check_for_dropped_samples(packet.sample.sample_index)) {
      // Dropped samples detected, don't process this sample
      break;
    }

    auto ros_sample = create_ros_sample(packet.sample, device_info);

    // Always handle the sample
    handle_sample(ros_sample);
    break;
  }

  case INTERNAL:
    RCLCPP_DEBUG(this->get_logger(), "Internal adapter packet received.");
    break;

  case ERROR:
    RCLCPP_ERROR(this->get_logger(), "Error reading data packet.");
    set_device_state(EegDeviceState::WAITING_FOR_EEG_DEVICE);
    break;

  case END:
    RCLCPP_INFO(this->get_logger(), "EEG device measurement stopped.");
    set_device_state(EegDeviceState::WAITING_FOR_EEG_DEVICE);
    break;

  default:
    RCLCPP_WARN(this->get_logger(), "Unknown result type while reading packet.");
  }
}

void EegBridge::spin() {
  auto base_interface = this->get_node_base_interface();

  try {
    while (rclcpp::ok()) {
      rclcpp::spin_some(base_interface);

      process_eeg_packet();

      check_for_sample_timeout();
    }
  } catch (const rclcpp::exceptions::RCLError &exception) {
    RCLCPP_ERROR(rclcpp::get_logger("eeg_bridge"), "Failed with %s", exception.what());
  }
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto logger = rclcpp::get_logger("eeg_bridge");

  realtime_utils::MemoryConfig mem_config;
  mem_config.enable_memory_optimization = true;
  mem_config.preallocate_size = 10 * 1024 * 1024; // 10 MB

  realtime_utils::SchedulingConfig sched_config;
  sched_config.enable_scheduling_optimization = true;
  sched_config.scheduling_policy = SCHED_RR;
  sched_config.priority_level = realtime_utils::PriorityLevel::HIGHEST_REALTIME;

  try {
    realtime_utils::initialize_scheduling(sched_config, logger);
    realtime_utils::initialize_memory(mem_config, logger);
  } catch (const std::exception& e) {
    RCLCPP_FATAL(logger, "Initialization failed: %s", e.what());
    return -1;
  }

  auto node = std::make_shared<EegBridge>();

  node->spin();
  rclcpp::shutdown();
  return 0;
}
