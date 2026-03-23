#ifndef MTMS_SIMULATOR__CHANNEL_HPP_
#define MTMS_SIMULATOR__CHANNEL_HPP_

#include <cstdint>
#include <mutex>

#include "rclcpp/rclcpp.hpp"

#include "mtms_event_interfaces/msg/charge_feedback.hpp"
#include "mtms_event_interfaces/msg/discharge_feedback.hpp"
#include "mtms_event_interfaces/msg/pulse_feedback.hpp"

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

  mtms_event_interfaces::msg::ChargeFeedback charge(uint16_t target_voltage, uint16_t event_id);
  mtms_event_interfaces::msg::DischargeFeedback discharge(uint16_t requested_target_voltage, uint16_t event_id);
  mtms_event_interfaces::msg::PulseFeedback pulse(uint16_t event_id, uint16_t duration_ticks);

  // Thread-safe state accessors. These are used by `/mtms/device/system_state` publishing.
  double current_voltage() const;
  uint16_t temperature() const;

  bool is_charging() const;
  bool is_discharging() const;
  bool is_pulse_in_progress() const;
  uint32_t pulse_count() const;
  mtms_device_interfaces::msg::ChannelError channel_error() const;

  void set_settings(const mtms_device_interfaces::msg::Settings & settings);

private:
  mutable std::mutex mutex_;

  rclcpp::Logger logger_;
  double charge_rate_ {0.0};
  double capacitance_ {0.0};
  double time_constant_ {0.0};
  double pulse_voltage_drop_proportion_ {0.0};
  uint16_t max_voltage_ {0};

  // Channel state (protected by mutex_).
  double current_voltage_ {0.0};
  uint16_t temperature_ {24};

  mtms_device_interfaces::msg::ChannelError errors_;
  bool is_charging_ {false};
  bool is_discharging_ {false};
  bool is_pulse_in_progress_ {false};
  uint32_t pulse_count_ {0};
  mtms_device_interfaces::msg::Settings settings_;
};

#endif  // MTMS_SIMULATOR__CHANNEL_HPP_
