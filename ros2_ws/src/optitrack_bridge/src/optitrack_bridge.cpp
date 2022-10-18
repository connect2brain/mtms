//
// Created by alqio on 18.10.2022.
//

#include "optitrack_bridge.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"


OptitrackBridge::OptitrackBridge() : Node("optitrack_bridge") {
  this->declare_parameter<int>("batch_size", 100);
  this->get_parameter("batch_size", batch_size);

  batch_publisher = this->create_publisher<mtms_interfaces::msg::EegBatch>("/eeg/batch_data", 10);
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<OptitrackBridge>();

  rclcpp::spin(node);

  rclcpp::shutdown();
  return 0;
}
