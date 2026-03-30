#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include "std_msgs/msg/empty.hpp"

#include "eeg_bridge.h"

#include "adapters/eeg_adapter.h"
#include "adapters/neurone_adapter.h"

using namespace std::chrono;
using namespace std::chrono_literals;

/* Publisher topics */
const std::string EEG_RAW_TOPIC = "/mtms/eeg/raw";
const std::string DEVICE_INFO_TOPIC = "/mtms/eeg_device/info";
const std::string DROPPED_SAMPLES_TOPIC = "/mtms/eeg_device/dropped_samples";
const std::string HEARTBEAT_TOPIC = "/mtms/eeg_bridge/heartbeat";
constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

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
      this->create_publisher<mtms_eeg_interfaces::msg::Sample>(EEG_RAW_TOPIC, EEG_QUEUE_LENGTH);

  this->device_info_publisher =
      this->create_publisher<mtms_eeg_interfaces::msg::EegDeviceInfo>(DEVICE_INFO_TOPIC, qos_persist_latest);

  this->dropped_samples_publisher =
    this->create_publisher<std_msgs::msg::Int32>(DROPPED_SAMPLES_TOPIC, 10);

  this->declare_parameter<int>("eeg_port", 50001);
  this->declare_parameter<std::string>("eeg_device", "neurone");

  const auto eeg_port_param = this->get_parameter("eeg_port").as_int();
  this->port = static_cast<uint16_t>(eeg_port_param);
  this->eeg_device_type = this->get_parameter("eeg_device").as_string();

  this->socket_ = std::make_shared<UdpSocket>(this->port);
  if (!this->socket_->init_socket()) {
    throw std::runtime_error("Failed to initialize UDP socket");
  }

  if (this->eeg_device_type == "neurone") {
    this->eeg_adapter = std::make_shared<NeurOneAdapter>(this->socket_);
  } else {
    throw std::runtime_error("Unsupported EEG device: " + this->eeg_device_type);
  }

  RCLCPP_INFO(this->get_logger(), "EEG bridge configuration:");
  RCLCPP_INFO(this->get_logger(), "  • Port: %u", this->port);
  RCLCPP_INFO(this->get_logger(), "  • EEG device: %s", this->eeg_device_type.c_str());
  RCLCPP_INFO(this->get_logger(), "  • Maximum dropped samples: %d", this->maximum_dropped_samples);
  publish_device_info();

  auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
  heartbeat_timer = this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
    heartbeat_publisher->publish(std_msgs::msg::Empty());
  });
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
  RCLCPP_INFO(this->get_logger(), "Resetting state...");

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

mtms_eeg_interfaces::msg::Sample EegBridge::create_ros_sample(const AdapterSample& adapter_sample,
                                                    const mtms_eeg_interfaces::msg::EegDeviceInfo& [[maybe_unused]] device_info) {
  auto sample = mtms_eeg_interfaces::msg::Sample();
  sample.eeg = adapter_sample.eeg;
  sample.emg = adapter_sample.emg;
  sample.time = adapter_sample.time;
  sample.trigger_a = adapter_sample.trigger_a;
  sample.trigger_b = adapter_sample.trigger_b;
  
  return sample;
}

void EegBridge::handle_sample(mtms_eeg_interfaces::msg::Sample sample) {
  /* If this is the first sample, set the time offset. */
  if (std::isnan(this->time_offset)) {
    this->time_offset = sample.time;
  }

  sample.time -= this->time_offset;

  /* Set the streaming sample index. */
  sample.sample_index = this->session_sample_index;
  this->session_sample_index++;

  /* Set the system time when the sample is published (nanoseconds since epoch). */
  auto now = std::chrono::high_resolution_clock::now();
  uint64_t system_time_eeg_bridge_published = std::chrono::duration_cast<std::chrono::nanoseconds>(
    now.time_since_epoch()).count();
  sample.system_time_eeg_bridge_published = system_time_eeg_bridge_published;

  this->eeg_sample_publisher->publish(sample);

  // Log latency trigger when present
  if (sample.trigger_b) {
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Receiving loopback triggers");
  }
  
  // Log pulse trigger when present
  if (sample.trigger_a) {
    RCLCPP_INFO(this->get_logger(), "Received TMS pulse at time: %.4f s", sample.time);
  }

  RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Streaming sample %lu at time: %.4f s", sample.sample_index, sample.time);
}

void EegBridge::process_eeg_packet() {
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

  auto node = std::make_shared<EegBridge>();

  node->spin();
  rclcpp::shutdown();
  return 0;
}
