//
// Created by alqio on 9/12/22.
//

#ifndef DATA_PROCESSOR_DATA_PROCESSOR_H
#define DATA_PROCESSOR_DATA_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/trigger.hpp"
#include "event_interfaces/msg/trigger_out.hpp"

#include <string>
#include <fstream>

class EEGTriggerProcessor : public rclcpp::Node {
public:
  EEGTriggerProcessor();

private:
  std::fstream f;

  std::vector<double> durations;
  uint32_t index;

  rclcpp::Subscription<eeg_interfaces::msg::Trigger>::SharedPtr trigger_subscription;

  rclcpp::Publisher<event_interfaces::msg::TriggerOut>::SharedPtr trigger_out_publisher;
};

#endif //DATA_PROCESSOR_DATA_PROCESSOR_H
