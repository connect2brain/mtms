//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_EEG_PREPROCESSOR_H
#define EEG_PROCESSOR_EEG_PREPROCESSOR_H

#include "eeg_interfaces/msg/eeg_datapoint.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"

#include "processor_node.h"

const uint16_t UNSET_SAMPLING_FREQUENCY = 0;
const double_t UNSET_PREVIOUS_TIME = std::numeric_limits<double_t>::quiet_NaN();

class EegPreprocessor : public ProcessorNode<eeg_interfaces::msg::EegDatapoint, eeg_interfaces::msg::EegDatapoint> {
public:
  EegPreprocessor();
private:
  void update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg);
  void check_dropped_samples(double_t current_time);
  void handle_eeg_datapoint(const std::shared_ptr<eeg_interfaces::msg::EegDatapoint> msg);

  virtual void publish_events(double_t time, const std::vector<Event> &events);
  void publish_cleaned_eeg(double_t time, const std::vector<eeg_interfaces::msg::EegDatapoint> &events);

  rclcpp::Subscription<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_subscription;
  rclcpp::Publisher<eeg_interfaces::msg::EegDatapoint>::SharedPtr cleaned_eeg_publisher;

  uint16_t sampling_frequency;
  double_t sampling_period;
  double_t previous_time;

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = pow(10, -5);
};

#endif //EEG_PROCESSOR_EEG_PREPROCESSOR_H
