#ifndef EEG_SIMULATOR_H
#define EEG_SIMULATOR_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/parameter_descriptor.hpp"
#include "rcl_interfaces/msg/parameter_type.hpp"

#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"
#include "eeg_interfaces/msg/trigger.hpp"

class EegSimulator : public rclcpp::Node {
public:
  EegSimulator();

private:
  void publish_sample();
  void publish_trigger();

  double current_time = 0.0;
  double sampling_period = 0.0;

  double last_log_time = -1.0;

  std::string data_file_name;
  std::string data_path;
  int sampling_frequency = 0;
  bool loop = false;
  int num_of_eeg_channels = 0;
  int num_of_emg_channels = 0;
  int total_channels = 0;
  std::ifstream file;

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
