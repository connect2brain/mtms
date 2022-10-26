//
// Created by alqio on 9/12/22.
//

#ifndef DATA_PROCESSOR_DATA_PROCESSOR_H
#define DATA_PROCESSOR_DATA_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "mtms_interfaces/msg/trigger.hpp"
#include "mtms_interfaces/msg/eeg_batch.hpp"

#include <string>
#include <fstream>

class DataBatcher : public rclcpp::Node {
public:
  DataBatcher();

private:
  std::vector<mtms_interfaces::msg::EegDatapoint> batch;
  unsigned int batch_index;
  unsigned int batch_size;
  unsigned int downsample_ratio;
  unsigned int send_counter;

  rclcpp::Subscription<mtms_interfaces::msg::EegDatapoint>::SharedPtr eeg_subscription;
  rclcpp::Publisher<mtms_interfaces::msg::EegBatch>::SharedPtr batch_publisher;
};

#endif //DATA_PROCESSOR_DATA_PROCESSOR_H
