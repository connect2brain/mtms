//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_PREPROCESSOR_H
#define EEG_PROCESSOR_PREPROCESSOR_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"

#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"

#include "project_interfaces/msg/preprocessor_list.hpp"
#include "project_interfaces/srv/set_preprocessor.hpp"

const uint16_t UNSET_SAMPLING_FREQUENCY = 0;
const double_t UNSET_PREVIOUS_TIME = std::numeric_limits<double_t>::quiet_NaN();

class EegPreprocessor : public rclcpp::Node {
public:
  EegPreprocessor();

private:
  void set_preprocessor(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessor::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessor::Response> response);

  void set_active_project(const std::shared_ptr<std_msgs::msg::String> msg);
  std::vector<std::string> list_python_scripts(const std::string& path);
  void update_preprocessor_list();

  void update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg);
  void check_dropped_samples(double_t current_time);
  void handle_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg);

  void publish_cleaned_eeg(double_t time, const std::vector<eeg_interfaces::msg::EegSample> &events);

  rclcpp::Subscription<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::EegSample>::SharedPtr eeg_raw_subscriber;
  rclcpp::Publisher<eeg_interfaces::msg::EegSample>::SharedPtr eeg_preprocessed_publisher;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr active_project_subscriber;
  rclcpp::Publisher<project_interfaces::msg::PreprocessorList>::SharedPtr preprocessor_list_publisher;

  rclcpp::Service<project_interfaces::srv::SetPreprocessor>::SharedPtr set_preprocessor_service;

  std::string active_project;
  std::string preprocessor;

  uint16_t sampling_frequency;
  double_t sampling_period;
  double_t previous_time;

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = pow(10, -5);
};

#endif //EEG_PROCESSOR_PREPROCESSOR_H
