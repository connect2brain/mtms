//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_EEG_PREPROCESSOR_H
#define EEG_PROCESSOR_EEG_PREPROCESSOR_H

#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "processor_node.h"

class EegPreprocessor : public ProcessorNode<mtms_interfaces::msg::EegDatapoint, mtms_interfaces::msg::EegDatapoint> {
public:
  EegPreprocessor();
private:
  virtual void publish_events(double_t time, const std::vector<mtms_interfaces::msg::EegDatapoint> &events);
  rclcpp::Publisher<mtms_interfaces::msg::EegDatapoint>::SharedPtr publisher;

};

#endif //EEG_PROCESSOR_EEG_PREPROCESSOR_H
