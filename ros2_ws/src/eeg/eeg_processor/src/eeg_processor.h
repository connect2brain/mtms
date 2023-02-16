//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_EEG_PROCESSOR_H
#define EEG_PROCESSOR_EEG_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/msg/trigger_out.hpp"
#include "event_interfaces/msg/pulse.hpp"
#include "event_interfaces/msg/charge.hpp"
#include "event_interfaces/msg/discharge.hpp"

#include "eeg_interfaces/msg/eeg_datapoint.hpp"

#include "event.h"
#include "processor_node.h"

class EegProcessor : public ProcessorNode<eeg_interfaces::msg::EegDatapoint, Event> {
public:
  EegProcessor();
private:
  virtual void publish_events(double_t time, const std::vector<Event> &events);
  rclcpp::Publisher<event_interfaces::msg::Charge>::SharedPtr charge_publisher;
  rclcpp::Publisher<event_interfaces::msg::Discharge>::SharedPtr discharge_publisher;
  rclcpp::Publisher<event_interfaces::msg::Pulse>::SharedPtr pulse_publisher;
  rclcpp::Publisher<event_interfaces::msg::TriggerOut>::SharedPtr trigger_out_publisher;
  rclcpp::Publisher<event_interfaces::msg::Stimulus>::SharedPtr stimulus_publisher;

};

#endif //EEG_PROCESSOR_EEG_PROCESSOR_H
