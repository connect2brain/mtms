//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_PREPROCESSOR_H
#define EEG_PROCESSOR_PREPROCESSOR_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"

#include "eeg_interfaces/msg/eeg_info.hpp"
#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/preprocessed_eeg_sample.hpp"
#include "eeg_interfaces/msg/trigger.hpp"

#include "event_interfaces/msg/pulse_feedback.hpp"

#include "mtms_device_interfaces/msg/system_state.hpp"

#include "project_interfaces/msg/preprocessor_list.hpp"
#include "project_interfaces/srv/set_preprocessor.hpp"
#include "project_interfaces/srv/set_preprocessor_enabled.hpp"

#include "ring_buffer.h"

const uint16_t UNSET_SAMPLING_FREQUENCY = 0;
const uint8_t UNSET_NUM_OF_CHANNELS = 255;
const double_t UNSET_PREVIOUS_TIME = std::numeric_limits<double_t>::quiet_NaN();
const std::string UNSET_STRING = "";

class PreprocessorWrapper;

class EegPreprocessor : public rclcpp::Node {
public:
  EegPreprocessor();

private:
  void reset_preprocessor_module();
  void reset_sample_buffer();

  void handle_system_state(const std::shared_ptr<mtms_device_interfaces::msg::SystemState> msg);

  void set_preprocessor(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessor::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessor::Response> response);

  void set_preprocessor_enabled(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessorEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessorEnabled::Response> response);

  void set_active_project(const std::shared_ptr<std_msgs::msg::String> msg);
  std::vector<std::string> list_python_scripts(const std::string& path);
  void update_preprocessor_list();

  void update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg);
  void check_dropped_samples(double_t sample_time);

  void handle_eeg_trigger(const std::shared_ptr<eeg_interfaces::msg::Trigger> msg);
  void handle_pulse_feedback(const std::shared_ptr<event_interfaces::msg::PulseFeedback> msg);
  bool was_pulse_given(double_t sample_time);

  void process_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg);

  rclcpp::Logger logger;

  rclcpp::Subscription<mtms_device_interfaces::msg::SystemState>::SharedPtr system_state_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::EegSample>::SharedPtr raw_eeg_subscriber;
  rclcpp::Publisher<eeg_interfaces::msg::PreprocessedEegSample>::SharedPtr preprocessed_eeg_publisher;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr active_project_subscriber;
  rclcpp::Publisher<project_interfaces::msg::PreprocessorList>::SharedPtr preprocessor_list_publisher;

  rclcpp::Service<project_interfaces::srv::SetPreprocessor>::SharedPtr set_preprocessor_service;

  rclcpp::Service<project_interfaces::srv::SetPreprocessorEnabled>::SharedPtr set_preprocessor_enabled_service;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr preprocessor_enabled_publisher;

  rclcpp::Subscription<event_interfaces::msg::PulseFeedback>::SharedPtr pulse_feedback_subscriber;
  rclcpp::Subscription<eeg_interfaces::msg::Trigger>::SharedPtr eeg_trigger_subscriber;

  bool preprocessor_enabled;

  std::string active_project;

  std::string script_directory  = UNSET_STRING;
  std::string module_name = UNSET_STRING;

  uint16_t sampling_frequency = UNSET_SAMPLING_FREQUENCY;
  uint8_t num_of_eeg_channels = UNSET_NUM_OF_CHANNELS;
  uint8_t num_of_emg_channels = UNSET_NUM_OF_CHANNELS;

  double_t sampling_period;

  double_t previous_time = UNSET_PREVIOUS_TIME;

  std::queue<double_t> pulse_execution_times;

  RingBuffer<std::shared_ptr<eeg_interfaces::msg::EegSample>> sample_buffer;

  std::unique_ptr<PreprocessorWrapper> preprocessor_wrapper;

  /* Keep track of the session state so that the sample buffer and the Python module can be re-initialized
     just once when the session is stopped. */
  mtms_device_interfaces::msg::SessionState session_state;

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = pow(10, -5);
};

#endif //EEG_PROCESSOR_PREPROCESSOR_H
