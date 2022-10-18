//
// Created by alqio on 18.10.2022.
//

#ifndef FPGA_BRIDGE_OPTITRACK_BRIDGE_H
#define FPGA_BRIDGE_OPTITRACK_BRIDGE_H

#include "optitrack_client.h"

class OptitrackBridge {
public:
  OptitrackBridge();
private:
  OptitrackClient client;
  rclcpp::Publisher<mtms_interfaces::msg::EegBatch>::SharedPtr batch_publisher;
};


#endif //FPGA_BRIDGE_OPTITRACK_BRIDGE_H
