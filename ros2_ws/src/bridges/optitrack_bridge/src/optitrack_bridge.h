//
// Created by alqio on 18.10.2022.
//

#ifndef FPGA_BRIDGE_OPTITRACK_BRIDGE_H
#define FPGA_BRIDGE_OPTITRACK_BRIDGE_H

#include "optitrack_client.h"
#include "rclcpp/rclcpp.hpp"
#include "neuronavigation_interfaces/msg/optitrack_poses.hpp"

#define MEASUREMENT_PROBE_RIGID_BODY_ID 999
#define HEAD_RIGID_BODY_ID 1003
#define COIL_RIGID_BODY_ID 1002

class OptitrackBridge : public rclcpp::Node {
public:
  OptitrackBridge();

  void shutdown();

  //has to be static for compatibility with NatNet libraries
  static void data_received_callback(sFrameOfMocapData *data, void *pUserData);

  //has to be static so it can be used in data_received_callback
  static rclcpp::Publisher<neuronavigation_interfaces::msg::OptitrackPoses>::SharedPtr publisher;
private:
  OptitrackClient client;
  int server_index;
};


#endif //FPGA_BRIDGE_OPTITRACK_BRIDGE_H
