//
// Created by alqio on 9/12/22.
//

#ifndef DATA_PROCESSOR_DATA_PROCESSOR_H
#define DATA_PROCESSOR_DATA_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "mtms_interfaces/msg/trigger.hpp"
#include "fpga_interfaces/srv/send_trigger_out_event.hpp"

#include <string>
#include <fstream>

class EEGTriggerProcessor : public rclcpp::Node {
public:
  EEGTriggerProcessor();

private:
  uint64_t first_trigger_time_us;
  std::fstream f;
  std::shared_ptr<fpga_interfaces::srv::SendTriggerOutEvent_Request_<std::allocator<void>>> req;

  std::vector<double> durations;
  uint32_t index;

  rclcpp::Subscription<mtms_interfaces::msg::Trigger>::SharedPtr trigger_subscription;
  rclcpp::Client<fpga_interfaces::srv::SendTriggerOutEvent>::SharedPtr trigger_out_client;
};

#endif //DATA_PROCESSOR_DATA_PROCESSOR_H
