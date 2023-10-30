//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_DECIDER_H
#define EEG_PROCESSOR_DECIDER_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"

#include "eeg_interfaces/msg/eeg_info.hpp"
#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/preprocessed_eeg_sample.hpp"

#include "event_interfaces/msg/event_trigger.hpp"

#include "project_interfaces/msg/decider_list.hpp"
#include "project_interfaces/srv/set_decider.hpp"
#include "project_interfaces/srv/set_decider_enabled.hpp"

#include "ring_buffer.h"

const uint16_t UNSET_SAMPLING_FREQUENCY = 0;
const uint8_t UNSET_NUM_OF_CHANNELS = 255;
const double_t UNSET_PREVIOUS_TIME = std::numeric_limits<double_t>::quiet_NaN();
const std::string UNSET_STRING = "";

class DeciderWrapper;

class EegDecider : public rclcpp::Node {
public:
  EegDecider();

private:
  void set_decider(
      const std::shared_ptr<project_interfaces::srv::SetDecider::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDecider::Response> response);

  void set_decider_enabled(
      const std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Response> response);

  void reset_decider_module();

  void set_active_project(const std::shared_ptr<std_msgs::msg::String> msg);
  std::vector<std::string> list_python_scripts(const std::string& path);
  void update_decider_list();

  void reset_sample_buffer();

  void update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg);
  void check_dropped_samples(double_t sample_time);

  void process_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample> msg);

  rclcpp::Subscription<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::PreprocessedEegSample>::SharedPtr preprocessed_eeg_subscriber;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr active_project_subscriber;
  rclcpp::Publisher<project_interfaces::msg::DeciderList>::SharedPtr decider_list_publisher;

  rclcpp::Service<project_interfaces::srv::SetDecider>::SharedPtr set_decider_service;

  rclcpp::Service<project_interfaces::srv::SetDeciderEnabled>::SharedPtr set_decider_enabled_service;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr decider_enabled_publisher;

  rclcpp::Publisher<event_interfaces::msg::EventTrigger>::SharedPtr event_trigger_publisher;

  bool decider_enabled;

  std::string active_project;

  std::string script_directory  = UNSET_STRING;
  std::string module_name = UNSET_STRING;

  uint16_t sampling_frequency = UNSET_SAMPLING_FREQUENCY;
  uint8_t num_of_eeg_channels = UNSET_NUM_OF_CHANNELS;
  uint8_t num_of_emg_channels = UNSET_NUM_OF_CHANNELS;

  double_t sampling_period;

  double_t previous_time = UNSET_PREVIOUS_TIME;

  std::queue<double_t> pulse_execution_times;

  RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample>> sample_buffer;

  std::unique_ptr<DeciderWrapper> decider_wrapper;

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = pow(10, -5);
};

#endif //EEG_PROCESSOR_DECIDER_H
