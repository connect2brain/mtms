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
#include "python_processor.h"
#include "matlab_processor.h"
#include "compiled_matlab_processor.h"
#include "processor.h"

#include <string>
#include <fstream>



class DataProcessor : public rclcpp::Node {
public:
  DataProcessor();

private:

  rclcpp::TimerBase::SharedPtr timer_;

  uint64_t first_trigger_time_us;
  std::fstream f;
  std::shared_ptr<fpga_interfaces::srv::SendTriggerOutEvent_Request_<std::allocator<void>>> req;

  fpga_interfaces::srv::SendTriggerOutEvent::Request trigger_out_request;
  rclcpp::Subscription<mtms_interfaces::msg::EegDatapoint>::SharedPtr eeg_data_subscription;
  rclcpp::Subscription<mtms_interfaces::msg::Trigger>::SharedPtr trigger_subscription;
  rclcpp::Client<fpga_interfaces::srv::SendTriggerOutEvent>::SharedPtr trigger_out_client;
};

#endif //DATA_PROCESSOR_DATA_PROCESSOR_H
