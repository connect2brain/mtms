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

#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"

#include "event.h"
#include "processor_node.h"

const uint16_t UNSET_SAMPLING_FREQUENCY = 0;
const double_t UNSET_PREVIOUS_TIME = std::numeric_limits<double_t>::quiet_NaN();

class EegProcessor : public ProcessorNode<eeg_interfaces::msg::EegSample, Event> {
public:
  EegProcessor();
private:
  void update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg);
  void check_dropped_samples(double_t current_time);
  void handle_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg);

  virtual void publish_events(double_t time, const std::vector<Event> &events);

  std::string eeg_topic;

  rclcpp::Subscription<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_subscription;

  rclcpp::Publisher<event_interfaces::msg::Charge>::SharedPtr charge_publisher;
  rclcpp::Publisher<event_interfaces::msg::Discharge>::SharedPtr discharge_publisher;
  rclcpp::Publisher<event_interfaces::msg::Pulse>::SharedPtr pulse_publisher;
  rclcpp::Publisher<event_interfaces::msg::TriggerOut>::SharedPtr trigger_out_publisher;
  rclcpp::Publisher<event_interfaces::msg::Stimulus>::SharedPtr stimulus_publisher;

  uint16_t sampling_frequency;
  double_t sampling_period;
  double_t previous_time;

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = pow(10, -5);
};

#endif //EEG_PROCESSOR_EEG_PROCESSOR_H
