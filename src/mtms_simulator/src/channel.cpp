#include "mtms_simulator/channel.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

#include "mtms_event_interfaces/msg/charge_error.hpp"
#include "mtms_event_interfaces/msg/discharge_error.hpp"
#include "mtms_event_interfaces/msg/pulse_error.hpp"

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
  // Deterministic defaults for ROS message members.
  errors_ = mtms_device_interfaces::msg::ChannelError();
  settings_ = mtms_device_interfaces::msg::Settings();
}

mtms_event_interfaces::msg::ChargeFeedback Channel::charge(const uint16_t target_voltage, const uint16_t event_id)
{
  const double charge_rate_constant = capacitance_ / (2.0 * charge_rate_);
  double t = 0.0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    t =
      charge_rate_constant *
      ((target_voltage * target_voltage) - (current_voltage_ * current_voltage_));
    is_charging_ = true;
  }

  if (t > 0.0) {
    std::this_thread::sleep_for(std::chrono::duration<double>(t));
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    is_charging_ = false;
    current_voltage_ = static_cast<double>(target_voltage);
  }

  mtms_event_interfaces::msg::ChargeFeedback feedback;
  feedback.id = event_id;
  feedback.error.value = mtms_event_interfaces::msg::ChargeError::NO_ERROR;
  return feedback;
}

mtms_event_interfaces::msg::DischargeFeedback Channel::discharge(
  const uint16_t requested_target_voltage,
  const uint16_t event_id)
{
  double target_voltage = 0.0;
  double t = 0.0;
  bool should_discharge = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    target_voltage = std::max<double>(requested_target_voltage, 3.0);
    if (current_voltage_ > target_voltage) {
      t = time_constant_ * std::log(current_voltage_ / target_voltage);
      is_discharging_ = true;
      should_discharge = true;
    }
  }

  if (should_discharge) {
    if (t > 0.0) {
      std::this_thread::sleep_for(std::chrono::duration<double>(t));
    }
    std::lock_guard<std::mutex> lock(mutex_);
    is_discharging_ = false;
    current_voltage_ = target_voltage;
  }

  mtms_event_interfaces::msg::DischargeFeedback feedback;
  feedback.id = event_id;
  feedback.error.value = mtms_event_interfaces::msg::DischargeError::NO_ERROR;
  return feedback;
}

mtms_event_interfaces::msg::PulseFeedback Channel::pulse(const uint16_t event_id, const uint16_t duration_ticks)
{
  double duration = 0.0;
  double after_pulse_voltage_proportion = 0.0;
  double start_voltage = 0.0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    ++pulse_count_;
    duration = duration_ticks / CLOCK_FREQUENCY_HZ;
    after_pulse_voltage_proportion = 1.0 - pulse_voltage_drop_proportion_;
    start_voltage = current_voltage_;
    is_pulse_in_progress_ = true;
  }

  if (duration > 0.0) {
    std::this_thread::sleep_for(std::chrono::duration<double>(duration));
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    is_pulse_in_progress_ = false;
    current_voltage_ = after_pulse_voltage_proportion * start_voltage;
  }

  mtms_event_interfaces::msg::PulseFeedback feedback;
  feedback.id = event_id;
  feedback.error.value = mtms_event_interfaces::msg::PulseError::NO_ERROR;
  return feedback;
}

double Channel::current_voltage() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return current_voltage_;
}

uint16_t Channel::temperature() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return temperature_;
}

bool Channel::is_charging() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return is_charging_;
}

bool Channel::is_discharging() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return is_discharging_;
}

bool Channel::is_pulse_in_progress() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return is_pulse_in_progress_;
}

uint32_t Channel::pulse_count() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return pulse_count_;
}

mtms_device_interfaces::msg::ChannelError Channel::channel_error() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return errors_;
}

void Channel::set_settings(const mtms_device_interfaces::msg::Settings & settings)
{
  std::lock_guard<std::mutex> lock(mutex_);
  settings_ = settings;
}
