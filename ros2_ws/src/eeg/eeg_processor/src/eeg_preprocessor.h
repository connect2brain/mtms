//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_EEG_PREPROCESSOR_H
#define EEG_PROCESSOR_EEG_PREPROCESSOR_H

#include "eeg_interfaces/msg/eeg_datapoint.hpp"
#include "processor_node.h"

class EegPreprocessor : public ProcessorNode<eeg_interfaces::msg::EegDatapoint, eeg_interfaces::msg::EegDatapoint> {
public:
  EegPreprocessor();
private:
  virtual void publish_events(double_t time, const std::vector<eeg_interfaces::msg::EegDatapoint> &events);
  virtual void experiment_state_callback(const std::shared_ptr<mtms_device_interfaces::msg::SystemState> message);

  rclcpp::Publisher<eeg_interfaces::msg::EegDatapoint>::SharedPtr publisher;

};

#endif //EEG_PROCESSOR_EEG_PREPROCESSOR_H
