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

#include "system_interfaces/msg/session.hpp"
#include "system_interfaces/msg/session_state.hpp"

#include "project_interfaces/msg/preprocessor_list.hpp"
#include "project_interfaces/srv/set_preprocessor_module.hpp"
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
  ~EegPreprocessor();

private:
  void initialize_preprocessor_module();
  void initialize_sample_buffer();

  void handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg);

  void unset_preprocessor_module();

  void set_preprocessor_module(const std::string module);
  void handle_set_preprocessor_module(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessorModule::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessorModule::Response> response);

  void handle_set_preprocessor_enabled(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessorEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessorEnabled::Response> response);

  void handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg);
  std::vector<std::string> list_python_modules(const std::string& path);
  void update_preprocessor_list();

  void update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg);
  void check_dropped_samples(double_t sample_time);

  void handle_eeg_trigger(const std::shared_ptr<eeg_interfaces::msg::Trigger> msg);
  void handle_pulse_feedback(const std::shared_ptr<event_interfaces::msg::PulseFeedback> msg);
  bool was_pulse_given(double_t sample_time);

  void process_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg);

  /* Inotify functions */
  void update_inotify_watch();
  void inotify_timer_callback();

  rclcpp::Logger logger;

  rclcpp::Subscription<system_interfaces::msg::Session>::SharedPtr session_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::EegSample>::SharedPtr raw_eeg_subscriber;
  rclcpp::Publisher<eeg_interfaces::msg::PreprocessedEegSample>::SharedPtr preprocessed_eeg_publisher;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr active_project_subscriber;
  rclcpp::Publisher<project_interfaces::msg::PreprocessorList>::SharedPtr preprocessor_list_publisher;

  rclcpp::Service<project_interfaces::srv::SetPreprocessorModule>::SharedPtr set_preprocessor_module_service;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr preprocessor_module_publisher;

  rclcpp::Service<project_interfaces::srv::SetPreprocessorEnabled>::SharedPtr set_preprocessor_enabled_service;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr preprocessor_enabled_publisher;

  rclcpp::Subscription<event_interfaces::msg::PulseFeedback>::SharedPtr pulse_feedback_subscriber;
  rclcpp::Subscription<eeg_interfaces::msg::Trigger>::SharedPtr eeg_trigger_subscriber;

  bool enabled;

  std::string active_project;

  std::string script_directory  = UNSET_STRING;
  std::string module_name = UNSET_STRING;

  std::vector<std::string> modules;

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
  system_interfaces::msg::SessionState session_state;

  /* Inotify variables */
  rclcpp::TimerBase::SharedPtr inotify_timer;
  int inotify_descriptor;
  int watch_descriptor;
  char inotify_buffer[1024];

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = pow(10, -5);
};

#endif //EEG_PROCESSOR_PREPROCESSOR_H
