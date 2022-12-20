//
// Created by alqio on 9/12/22.
//

#ifndef DATA_PROCESSOR_DATA_PROCESSOR_H
#define DATA_PROCESSOR_DATA_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "mtms_interfaces/msg/trigger.hpp"
#include "fpga_interfaces/srv/send_signal_out.hpp"

#include <string>
#include <fstream>

class EEGTriggerProcessor : public rclcpp::Node {
public:
  EEGTriggerProcessor();

private:
  std::fstream f;
  std::shared_ptr<fpga_interfaces::srv::SendSignalOut_Request_<std::allocator<void>>> req;

  std::vector<double> durations;
  uint32_t index;

  rclcpp::Subscription<mtms_interfaces::msg::Trigger>::SharedPtr trigger_subscription;
  rclcpp::Client<fpga_interfaces::srv::SendSignalOut>::SharedPtr signal_out_client;
};

#endif //DATA_PROCESSOR_DATA_PROCESSOR_H
