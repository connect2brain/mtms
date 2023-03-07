//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_STIMULUS_PRESENTER_H
#define EEG_PROCESSOR_STIMULUS_PRESENTER_H

#include "event_interfaces/msg/trigger_out.hpp"
#include "eeg_interfaces/msg/eeg_datapoint.hpp"
#include "processor_node.h"

class StimulusPresenter : public ProcessorNode<event_interfaces::msg::Stimulus, Event> {
public:
  StimulusPresenter();
private:
  virtual void publish_events(double_t time, const std::vector<Event> &events);

  rclcpp::Publisher<event_interfaces::msg::Charge>::SharedPtr charge_publisher;
  rclcpp::Publisher<event_interfaces::msg::Discharge>::SharedPtr discharge_publisher;
  rclcpp::Publisher<event_interfaces::msg::Pulse>::SharedPtr pulse_publisher;
  rclcpp::Publisher<event_interfaces::msg::TriggerOut>::SharedPtr trigger_out_publisher;

  rclcpp::Subscription<eeg_interfaces::msg::EegDatapoint>::SharedPtr eeg_subscription;

  std::vector<event_interfaces::msg::Stimulus> event_buffer;

};

#endif //EEG_PROCESSOR_STIMULUS_PRESENTER_H
