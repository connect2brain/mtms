//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_STIMULUS_PRESENTER_H
#define EEG_PROCESSOR_STIMULUS_PRESENTER_H

#include "fpga_interfaces/msg/signal_out.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "processor_node.h"

class StimulusPresenter : public ProcessorNode<mtms_interfaces::msg::Event, Event> {
public:
  StimulusPresenter();
private:
  virtual void publish_events(double_t time, const std::vector<Event> &events);
  rclcpp::Publisher<fpga_interfaces::msg::Charge>::SharedPtr charge_publisher;
  rclcpp::Publisher<fpga_interfaces::msg::Discharge>::SharedPtr discharge_publisher;
  rclcpp::Publisher<fpga_interfaces::msg::Pulse>::SharedPtr pulse_publisher;
  rclcpp::Publisher<fpga_interfaces::msg::SignalOut>::SharedPtr signal_out_publisher;

};

#endif //EEG_PROCESSOR_STIMULUS_PRESENTER_H
