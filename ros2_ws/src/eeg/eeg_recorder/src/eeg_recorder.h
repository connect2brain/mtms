#ifndef EEG_RECORDER_H
#define EEG_RECORDER_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"

#include "eeg_interfaces/msg/sample.hpp"
#include "eeg_interfaces/msg/preprocessed_sample.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"

#include "system_interfaces/msg/session.hpp"
#include "system_interfaces/msg/session_state.hpp"


class EegRecorder : public rclcpp::Node {
public:
  EegRecorder();

private:
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr active_project_subscriber;

  rclcpp::Subscription<system_interfaces::msg::Session>::SharedPtr session_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::Sample>::SharedPtr eeg_raw_subscriber;
  rclcpp::Subscription<eeg_interfaces::msg::PreprocessedSample>::SharedPtr eeg_preprocessed_subscriber;

  void handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg);
  void handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg);

  void handle_raw_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::Sample> msg);
  void handle_preprocessed_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::PreprocessedSample> msg);

  void write_raw_buffer();
  void write_preprocessed_buffer();

  std::string active_project;

  std::string experiment_name;
  std::string subject_name;

  std::string filename;

  std::string raw_data_directory;
  std::string raw_file_path;
  std::ofstream raw_file;
  uint64_t raw_sample_count = 0;
  std::ostringstream raw_buffer;

  std::string preprocessed_data_directory;
  std::string preprocessed_file_path;
  std::ofstream preprocessed_file;
  uint64_t preprocessed_sample_count = 0;
  std::ostringstream preprocessed_buffer;

  uint8_t current_session_state;
};

#endif //EEG_RECORDER_H
