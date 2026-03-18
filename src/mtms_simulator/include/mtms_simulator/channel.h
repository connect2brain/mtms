#ifndef MTMS_SIMULATOR__CHANNEL_HPP_
#define MTMS_SIMULATOR__CHANNEL_HPP_

#include <cstdint>

#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/msg/charge_feedback.hpp"
#include "event_interfaces/msg/discharge_feedback.hpp"
#include "event_interfaces/msg/pulse_feedback.hpp"

#include "mtms_device_interfaces/msg/channel_error.hpp"
#include "mtms_device_interfaces/msg/settings.hpp"

class Channel {
public:
  static constexpr double CLOCK_FREQUENCY_HZ = 4e7;

  Channel(
    double charge_rate,
    double capacitance,
    double time_constant,
    double pulse_voltage_drop_proportion,
    uint16_t max_voltage,
    const rclcpp::Logger & logger);

  event_interfaces::msg::ChargeFeedback charge(uint16_t target_voltage, uint16_t event_id);
  event_interfaces::msg::DischargeFeedback discharge(uint16_t requested_target_voltage, uint16_t event_id);
  event_interfaces::msg::PulseFeedback pulse(uint16_t event_id, uint16_t duration_ticks);

  double current_voltage() const;
  uint16_t temperature() const;

  mtms_device_interfaces::msg::ChannelError errors;
  bool is_charging {false};
  bool is_discharging {false};
  bool is_pulse_in_progress {false};
  bool allow_stimulation {false};
  uint32_t pulse_count {0};
  mtms_device_interfaces::msg::Settings settings;

private:
  rclcpp::Logger logger_;
  double charge_rate_ {0.0};
  double capacitance_ {0.0};
  double time_constant_ {0.0};
  double pulse_voltage_drop_proportion_ {0.0};
  uint16_t max_voltage_ {0};
  double current_voltage_ {0.0};
  uint16_t temperature_ {24};
};

#endif  // MTMS_SIMULATOR__CHANNEL_HPP_
