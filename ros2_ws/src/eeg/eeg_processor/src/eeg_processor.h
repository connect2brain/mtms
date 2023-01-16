//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_EEG_PROCESSOR_H
#define EEG_PROCESSOR_EEG_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/signal_out.hpp"
#include "fpga_interfaces/msg/pulse.hpp"
#include "fpga_interfaces/msg/charge.hpp"
#include "fpga_interfaces/msg/discharge.hpp"

#include "mtms_interfaces/msg/eeg_datapoint.hpp"

#include "fpga_event.h"

#include "processor_node.h"

class EegProcessor : ProcessorNode<mtms_interfaces::msg::EegDatapoint, mtms_interfaces::msg::EegDatapoint, Event> {
public:
  EegProcessor();
private:
  virtual void publish_events(double_t time, const std::vector<Event> &events);
  rclcpp::Publisher<fpga_interfaces::msg::Charge>::SharedPtr charge_publisher;
  rclcpp::Publisher<fpga_interfaces::msg::Discharge>::SharedPtr discharge_publisher;
  rclcpp::Publisher<fpga_interfaces::msg::Pulse>::SharedPtr pulse_publisher;
  rclcpp::Publisher<fpga_interfaces::msg::SignalOut>::SharedPtr signal_out_publisher;

};

#endif //EEG_PROCESSOR_EEG_PROCESSOR_H
