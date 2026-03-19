#ifndef MEP_ANALYZER__MEP_ANALYZER_H_
#define MEP_ANALYZER__MEP_ANALYZER_H_

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "eeg_interfaces/msg/eeg_device_info.hpp"
#include "eeg_interfaces/msg/sample.hpp"
#include "mep_interfaces/srv/analyze_mep.hpp"

#include "ring_buffer.h"

class MepAnalyzerNode : public rclcpp::Node {
public:
  explicit MepAnalyzerNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
  static const std::string EEG_RAW_TOPIC;
  static const std::string EEG_DEVICE_INFO_TOPIC;
  static const std::string SERVICE_ANALYZE_MEP;

  static constexpr double eeg_bufferWINDOW_S = 3.0;
  static constexpr double WAIT_POLL_PERIOD_S = 0.01;

  void device_info_callback(const eeg_interfaces::msg::EegDeviceInfo::SharedPtr msg);
  void eeg_sample_callback(const eeg_interfaces::msg::Sample::SharedPtr msg);

  void analyze_mep_handler(
    const std::shared_ptr<mep_interfaces::srv::AnalyzeMep::Request> request,
    std::shared_ptr<mep_interfaces::srv::AnalyzeMep::Response> response);

  bool wait_for_next_stimulation_time(double & stimulation_time_s);
  bool wait_until_buffer_covers(double start_time_s, double end_time_s);

  bool extract_emg_window(
    uint8_t emg_channel,
    double start_time_s,
    double end_time_s,
    std::vector<double> & out_emg) const;

  static double max_minus_min(const std::vector<double> & v);

  // Keep EEG streaming callbacks sequential to preserve strict sample_index order.
  rclcpp::CallbackGroup::SharedPtr data_callback_group;
  // Allow the service handler to run concurrently with EEG ingestion.
  rclcpp::CallbackGroup::SharedPtr service_callback_group;
  rclcpp::Subscription<eeg_interfaces::msg::EegDeviceInfo>::SharedPtr device_info_subscriber;
  rclcpp::Subscription<eeg_interfaces::msg::Sample>::SharedPtr eeg_subscriber;
  rclcpp::Service<mep_interfaces::srv::AnalyzeMep>::SharedPtr analyze_mep_service;

  mutable std::mutex buffer_mutex;
  std::condition_variable buffer_cv;
  RingBuffer<eeg_interfaces::msg::Sample> eeg_buffer;

  std::optional<uint32_t> sampling_frequency_hz;
  std::optional<uint64_t> last_sample_index;
  std::optional<double> last_trigger_a_time_s;
  uint64_t samples_dropped_counter {0};
};

#endif  // MEP_ANALYZER__MEP_ANALYZER_H_
