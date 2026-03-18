#include "mtms_simulator/channel.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

#include "event_msgs/msg/charge_error.hpp"
#include "event_msgs/msg/discharge_error.hpp"
#include "event_msgs/msg/pulse_error.hpp"

Channel::Channel(
  const double charge_rate,
  const double capacitance,
  const double time_constant,
  const double pulse_voltage_drop_proportion,
  const uint16_t max_voltage,
  const rclcpp::Logger & logger)
: logger_(logger),
  charge_rate_(charge_rate),
  capacitance_(capacitance),
  time_constant_(time_constant),
  pulse_voltage_drop_proportion_(pulse_voltage_drop_proportion),
  max_voltage_(max_voltage)
{
  (void)max_voltage_;
}

event_msgs::msg::ChargeFeedback Channel::charge(const uint16_t target_voltage, const uint16_t event_id)
{
  RCLCPP_INFO(
    logger_, "Charge requested: from %.2fV to %uV.",
    current_voltage_, target_voltage);

  const double charge_rate_constant = capacitance_ / (2.0 * charge_rate_);
  const double t =
    charge_rate_constant *
    ((target_voltage * target_voltage) - (current_voltage_ * current_voltage_));

  is_charging = true;
  if (t > 0.0) {
    std::this_thread::sleep_for(std::chrono::duration<double>(t));
  }
  is_charging = false;

  current_voltage_ = static_cast<double>(target_voltage);

  event_msgs::msg::ChargeFeedback feedback;
  feedback.id = event_id;
  feedback.error.value = event_msgs::msg::ChargeError::NO_ERROR;
  return feedback;
}

event_msgs::msg::DischargeFeedback Channel::discharge(
  const uint16_t requested_target_voltage,
  const uint16_t event_id)
{
  const double target_voltage = std::max<double>(requested_target_voltage, 3.0);

  RCLCPP_INFO(
    logger_, "Discharge requested: from %.2fV to %uV.",
    current_voltage_, requested_target_voltage);

  if (current_voltage_ <= target_voltage) {
    event_msgs::msg::DischargeFeedback feedback;
    feedback.id = event_id;
    feedback.error.value = event_msgs::msg::DischargeError::NO_ERROR;
    return feedback;
  }

  const double t = time_constant_ * std::log(current_voltage_ / target_voltage);

  is_discharging = true;
  if (t > 0.0) {
    std::this_thread::sleep_for(std::chrono::duration<double>(t));
  }
  is_discharging = false;

  current_voltage_ = target_voltage;

  event_msgs::msg::DischargeFeedback feedback;
  feedback.id = event_id;
  feedback.error.value = event_msgs::msg::DischargeError::NO_ERROR;
  return feedback;
}

event_msgs::msg::PulseFeedback Channel::pulse(const uint16_t event_id, const uint16_t duration_ticks)
{
  const double next_voltage =
    current_voltage_ - pulse_voltage_drop_proportion_ * current_voltage_;
  RCLCPP_INFO(
    logger_, "Pulse requested: from %.2fV to %.2fV.",
    current_voltage_, next_voltage);

  ++pulse_count;
  const double duration = duration_ticks / CLOCK_FREQUENCY_HZ;

  is_pulse_in_progress = true;
  if (duration > 0.0) {
    std::this_thread::sleep_for(std::chrono::duration<double>(duration));
  }
  is_pulse_in_progress = false;

  const double after_pulse_voltage_proportion = 1.0 - pulse_voltage_drop_proportion_;
  current_voltage_ = after_pulse_voltage_proportion * current_voltage_;

  event_msgs::msg::PulseFeedback feedback;
  feedback.id = event_id;
  feedback.error.value = event_msgs::msg::PulseError::NO_ERROR;
  return feedback;
}

double Channel::current_voltage() const
{
  return current_voltage_;
}

uint16_t Channel::temperature() const
{
  return temperature_;
}
