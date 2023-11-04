#ifndef EEG_SIMULATOR_H
#define EEG_SIMULATOR_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/parameter_descriptor.hpp"
#include "rcl_interfaces/msg/parameter_type.hpp"

#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"
#include "eeg_interfaces/msg/trigger.hpp"

#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"

#include "project_interfaces/msg/dataset.hpp"
#include "project_interfaces/msg/dataset_list.hpp"
#include "project_interfaces/srv/set_dataset.hpp"
#include "project_interfaces/srv/set_playback.hpp"
#include "project_interfaces/srv/set_loop.hpp"


class EegSimulator : public rclcpp::Node {
public:
  EegSimulator();
  ~EegSimulator();

private:
  std::vector<project_interfaces::msg::Dataset> list_datasets(const std::string& path);
  void update_dataset_list();
  void handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg);

  void handle_set_dataset(
      const std::shared_ptr<project_interfaces::srv::SetDataset::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDataset::Response> response);

  void handle_set_playback(
      const std::shared_ptr<project_interfaces::srv::SetPlayback::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPlayback::Response> response);

  void handle_set_loop(
      const std::shared_ptr<project_interfaces::srv::SetLoop::Request> request,
      std::shared_ptr<project_interfaces::srv::SetLoop::Response> response);

  void publish_sample();
  void publish_trigger();

  void update_inotify_watch();
  void inotify_timer_callback();

  std::string dataset_filename;

  bool playback;
  bool loop;

  double current_time = 0.0;
  double sampling_period = 0.0;

  double last_log_time = -1.0;

  std::string data_file_name;
  std::string data_path;
  int sampling_frequency = 0;
  int num_of_eeg_channels = 0;
  int num_of_emg_channels = 0;
  int total_channels = 0;
  std::ifstream file;

  std::string active_project;
  std::string dataset_directory;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr active_project_subscriber;
  rclcpp::Publisher<project_interfaces::msg::DatasetList>::SharedPtr dataset_list_publisher;

  rclcpp::Service<project_interfaces::srv::SetDataset>::SharedPtr set_dataset_service;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr dataset_publisher;

  rclcpp::Service<project_interfaces::srv::SetPlayback>::SharedPtr set_playback_service;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr playback_publisher;

  rclcpp::Service<project_interfaces::srv::SetLoop>::SharedPtr set_loop_service;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr loop_publisher;

  rclcpp::Publisher<eeg_interfaces::msg::EegSample>::SharedPtr eeg_publisher_;
  rclcpp::Publisher<eeg_interfaces::msg::Trigger>::SharedPtr trigger_publisher_;
  rclcpp::Publisher<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_publisher_;

  rclcpp::TimerBase::SharedPtr publish_sample_timer_;
  rclcpp::TimerBase::SharedPtr publish_trigger_timer_;

  /* Inotify variables */
  rclcpp::TimerBase::SharedPtr inotify_timer;
  int inotify_descriptor;
  int watch_descriptor;
  char inotify_buffer[1024];
};

#endif //EEG_SIMULATOR_H
