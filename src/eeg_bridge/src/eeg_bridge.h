#ifndef EEG_BRIDGE_H
#define EEG_BRIDGE_H

#include <cstdlib>
#include <netinet/in.h>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/int32.hpp"

#include "mtms_eeg_interfaces/msg/eeg_device_info.hpp"
#include "mtms_eeg_interfaces/msg/sample.hpp"

#include "adapters/eeg_adapter.h"
#include "adapters/socket.h"

using namespace std::chrono_literals;

enum EegDeviceState {
  WAITING_FOR_EEG_DEVICE,
  EEG_DEVICE_STREAMING
};

const double_t UNSET_TIME = std::numeric_limits<double_t>::quiet_NaN();
const uint64_t UNSET_PREVIOUS_SAMPLE_INDEX = std::numeric_limits<uint64_t>::max();

/**
 * Translate data from EEG adapter interface to ROS messages.
 *
 * Follows the adapter structure where class implementing EegAdapter interface
 * will be used to translate the raw socket data from the EEG device into common
 * format.
 *
 * Currently supported EEG devices are:
 *   - Bittium NeurOne
 */
class EegBridge : public rclcpp::Node {

public:
  EegBridge();

  void spin();

private:
  bool reset_state();

  void process_eeg_packet();

  mtms_eeg_interfaces::msg::Sample create_ros_sample(const AdapterSample& adapter_sample,
                                          const mtms_eeg_interfaces::msg::EegDeviceInfo& device_info);

  void handle_sample(mtms_eeg_interfaces::msg::Sample sample);
  bool check_for_dropped_samples(uint64_t device_sample_index);
  void check_for_sample_timeout();

  void publish_device_info();
  void publish_cumulative_dropped_samples();

  void set_device_state(EegDeviceState new_state);

  /* Configuration */
  uint16_t port = 0;
  uint8_t maximum_dropped_samples = 2;
  std::string eeg_device_type;

  std::shared_ptr<UdpSocket> socket_;
  std::shared_ptr<EegAdapter> eeg_adapter;
  uint8_t buffer[BUFFER_SIZE] = {0};

  /* State */
  EegDeviceState device_state = EegDeviceState::WAITING_FOR_EEG_DEVICE;
  std::chrono::steady_clock::time_point last_sample_time;

  /* Publishers */
  rclcpp::Publisher<mtms_eeg_interfaces::msg::Sample>::SharedPtr eeg_sample_publisher;
  rclcpp::Publisher<mtms_eeg_interfaces::msg::EegDeviceInfo>::SharedPtr device_info_publisher;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr dropped_samples_publisher;

  /* Device sample tracking for dropped sample detection */
  uint64_t previous_device_sample_index = UNSET_PREVIOUS_SAMPLE_INDEX;
  uint64_t cumulative_dropped_samples = 0;
  
  /* Streaming sample index (starts at 0 for each streaming run) */
  uint64_t session_sample_index = 0;

  double_t time_offset = UNSET_TIME;     // in seconds

  rclcpp::TimerBase::SharedPtr heartbeat_timer;
};

#endif
