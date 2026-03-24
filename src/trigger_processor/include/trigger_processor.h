#ifndef trigger_processor__trigger_processor_H_
#define trigger_processor__trigger_processor_H_

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "mtms_eeg_interfaces/msg/eeg_device_info.hpp"
#include "mtms_eeg_interfaces/msg/sample.hpp"
#include "mtms_trigger_interfaces/srv/analyze_mep.hpp"
#include "mtms_trigger_interfaces/srv/get_trigger_window.hpp"

#include "ring_buffer.h"

class TriggerProcessorNode : public rclcpp::Node {
public:
  explicit TriggerProcessorNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
  static const std::string EEG_RAW_TOPIC;
  static const std::string EEG_DEVICE_INFO_TOPIC;
  static const std::string SERVICE_ANALYZE_MEP;
  static const std::string SERVICE_GET_TRIGGER_WINDOW;

  static constexpr double EEG_BUFFER_WINDOW_S = 3.0;
  static constexpr double WAIT_POLL_PERIOD_S = 0.01;
  static constexpr double ANALYZE_MEP_TIMEOUT_S = 10.0;

  void device_info_callback(const mtms_eeg_interfaces::msg::EegDeviceInfo::SharedPtr msg);
  void eeg_sample_callback(const mtms_eeg_interfaces::msg::Sample::SharedPtr msg);

  void analyze_mep_handler(
    const std::shared_ptr<mtms_trigger_interfaces::srv::AnalyzeMep::Request> request,
    std::shared_ptr<mtms_trigger_interfaces::srv::AnalyzeMep::Response> response);

  void get_trigger_window_handler(
    const std::shared_ptr<mtms_trigger_interfaces::srv::GetTriggerWindow::Request> request,
    std::shared_ptr<mtms_trigger_interfaces::srv::GetTriggerWindow::Response> response);

  bool wait_for_next_stimulation_time(double & stimulation_time_s, double timeout_s);
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
  rclcpp::Subscription<mtms_eeg_interfaces::msg::EegDeviceInfo>::SharedPtr device_info_subscriber;
  rclcpp::Subscription<mtms_eeg_interfaces::msg::Sample>::SharedPtr eeg_subscriber;
  rclcpp::Service<mtms_trigger_interfaces::srv::AnalyzeMep>::SharedPtr analyze_mep_service;
  rclcpp::Service<mtms_trigger_interfaces::srv::GetTriggerWindow>::SharedPtr get_trigger_window_service;

  mutable std::mutex buffer_mutex;
  std::condition_variable buffer_cv;
  RingBuffer<mtms_eeg_interfaces::msg::Sample> eeg_buffer;

  std::optional<uint32_t> sampling_frequency_hz;
  std::optional<uint64_t> last_sample_index;
  std::optional<double> last_trigger_a_time_s;
  uint64_t samples_dropped_counter {0};

  rclcpp::TimerBase::SharedPtr heartbeat_timer;
};

#endif  // trigger_processor__trigger_processor_H_
