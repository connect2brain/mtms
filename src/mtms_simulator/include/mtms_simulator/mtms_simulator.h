#ifndef MTMS_SIMULATOR__MTMS_SIMULATOR_HPP_
#define MTMS_SIMULATOR__MTMS_SIMULATOR_HPP_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "event_msgs/msg/charge.hpp"
#include "event_msgs/msg/charge_feedback.hpp"
#include "event_msgs/msg/discharge.hpp"
#include "event_msgs/msg/discharge_feedback.hpp"
#include "event_msgs/msg/execution_condition.hpp"
#include "event_msgs/msg/pulse.hpp"
#include "event_msgs/msg/pulse_error.hpp"
#include "event_msgs/msg/pulse_feedback.hpp"
#include "event_msgs/msg/trigger_out.hpp"
#include "event_msgs/msg/trigger_out_feedback.hpp"

#include "mtms_device_interfaces/msg/device_state.hpp"
#include "mtms_device_interfaces/msg/settings.hpp"
#include "mtms_device_interfaces/msg/system_state.hpp"
#include "mtms_device_interfaces/srv/request_events.hpp"
#include "mtms_device_interfaces/srv/send_settings.hpp"
#include "mtms_device_interfaces/srv/start_device.hpp"
#include "mtms_device_interfaces/srv/stop_device.hpp"

#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/session.hpp"
#include "system_interfaces/msg/session_state.hpp"
#include "system_interfaces/srv/start_session.hpp"
#include "system_interfaces/srv/stop_session.hpp"

#include "waveform_msgs/msg/waveform.hpp"

#include "mtms_simulator/channel.h"

class MTMSSimulator : public rclcpp::Node {
public:
  MTMSSimulator();

private:
  static constexpr int SESSION_PUBLISHING_INTERVAL_MS = 1000;
  static constexpr int SESSION_PUBLISHING_INTERVAL_TOLERANCE_MS = 10;
  static constexpr int SYSTEM_STATE_PUBLISHING_INTERVAL_MS = 1000;
  static constexpr int SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE_MS = 10;
  static constexpr int HEALTHCHECK_PUBLISHING_INTERVAL_MS = 800;

  static constexpr double CAPACITANCE = 1020e-6;
  static constexpr double TIME_CONSTANT = 1.0;
  static constexpr double PULSE_VOLTAGE_DROP_PROPORTION = 0.05;

  static constexpr uint16_t DEFAULT_MAXIMUM_NUMBER_OF_PULSE_PIECES = 8;
  static constexpr uint16_t DEFAULT_MAXIMUM_RISING_FALLING_DIFFERENCE_TICKS = 10000;
  static constexpr uint16_t DEFAULT_MAXIMUM_PULSE_DURATION_TICKS = 10000;
  static constexpr uint8_t DEFAULT_PULSES_IN_MAXIMUM_PULSES_PER_TIME = 20;
  static constexpr uint16_t DEFAULT_TIME_IN_MAXIMUM_PULSES_PER_TIME_MS = 1000;

  static constexpr uint64_t UINT8_MAX_V = std::numeric_limits<uint8_t>::max();
  static constexpr uint64_t UINT16_MAX_V = std::numeric_limits<uint16_t>::max();
  static constexpr uint64_t UINT32_MAX_V = std::numeric_limits<uint32_t>::max();

  template<typename T>
  uint64_t clamp_uint(const T value, const uint64_t max_value) const
  {
    const auto rounded = static_cast<long long>(std::llround(static_cast<double>(value)));
    const auto clamped = std::clamp<long long>(rounded, 0, static_cast<long long>(max_value));
    return static_cast<uint64_t>(clamped);
  }

  void log_settings(const mtms_device_interfaces::msg::Settings & settings);
  void allow_stimulation_callback(const std_msgs::msg::Bool::SharedPtr msg);
  void allow_trigger_out_callback(const std_msgs::msg::Bool::SharedPtr msg);

  void send_settings_handler(
    const std::shared_ptr<mtms_device_interfaces::srv::SendSettings::Request> request,
    std::shared_ptr<mtms_device_interfaces::srv::SendSettings::Response> response);
  void start_device_handler(
    const std::shared_ptr<mtms_device_interfaces::srv::StartDevice::Request> request,
    std::shared_ptr<mtms_device_interfaces::srv::StartDevice::Response> response);
  void stop_device_handler(
    const std::shared_ptr<mtms_device_interfaces::srv::StopDevice::Request> request,
    std::shared_ptr<mtms_device_interfaces::srv::StopDevice::Response> response);
  void start_session_handler(
    const std::shared_ptr<system_interfaces::srv::StartSession::Request> request,
    std::shared_ptr<system_interfaces::srv::StartSession::Response> response);
  void stop_session_handler(
    const std::shared_ptr<system_interfaces::srv::StopSession::Request> request,
    std::shared_ptr<system_interfaces::srv::StopSession::Response> response);
  void request_events_handler(
    const std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Request> request,
    std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Response> response);
  void trigger_events_handler(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response);

  bool session_not_started() const;
  bool validate_charge_or_discharge(
    uint8_t channel, uint16_t target_voltage,
    uint8_t & error_value) const;
  void wait_for_execution_condition(
    const event_msgs::msg::ExecutionCondition & execution_condition,
    double execution_time) const;
  std::tuple<uint32_t, uint32_t, uint32_t> calculate_waveform_durations(
    const waveform_msgs::msg::Waveform & waveform) const;
  event_msgs::msg::PulseError validate_pulse(const event_msgs::msg::Pulse & message) const;

  void process_charge(const event_msgs::msg::Charge & message);
  void process_discharge(const event_msgs::msg::Discharge & message);
  void process_pulse(const event_msgs::msg::Pulse & message);
  void process_trigger_out(const event_msgs::msg::TriggerOut & message) const;

  void system_state_callback();
  void session_callback();
  void healthcheck_callback() const;

  size_t num_of_channels_ {5};
  uint16_t max_voltage_ {1500};
  double charge_rate_ {1500.0};

  bool allow_stimulation_ {false};
  bool allow_trigger_out_ {true};
  double session_start_time_ {0.0};

  mtms_device_interfaces::msg::SystemState system_state_;
  system_interfaces::msg::Session session_;
  mtms_device_interfaces::msg::Settings settings_;
  std::vector<Channel> channels_;

  rclcpp::Service<mtms_device_interfaces::srv::SendSettings>::SharedPtr send_settings_service_;
  rclcpp::Service<mtms_device_interfaces::srv::StartDevice>::SharedPtr start_device_service_;
  rclcpp::Service<mtms_device_interfaces::srv::StopDevice>::SharedPtr stop_device_service_;
  rclcpp::Service<system_interfaces::srv::StartSession>::SharedPtr start_session_service_;
  rclcpp::Service<system_interfaces::srv::StopSession>::SharedPtr stop_session_service_;
  rclcpp::Service<mtms_device_interfaces::srv::RequestEvents>::SharedPtr request_events_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr trigger_events_service_;

  rclcpp::Publisher<event_msgs::msg::PulseFeedback>::SharedPtr pulse_feedback_publisher_;
  rclcpp::Publisher<event_msgs::msg::ChargeFeedback>::SharedPtr charge_feedback_publisher_;
  rclcpp::Publisher<event_msgs::msg::DischargeFeedback>::SharedPtr discharge_feedback_publisher_;
  rclcpp::Publisher<event_msgs::msg::TriggerOutFeedback>::SharedPtr trigger_out_feedback_publisher_;
  rclcpp::Publisher<mtms_device_interfaces::msg::SystemState>::SharedPtr system_state_publisher_;
  rclcpp::Publisher<system_interfaces::msg::Session>::SharedPtr session_publisher_;
  rclcpp::Publisher<system_interfaces::msg::Healthcheck>::SharedPtr healthcheck_publisher_;

  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr allow_stimulation_subscription_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr allow_trigger_out_subscription_;

  rclcpp::TimerBase::SharedPtr session_timer_;
  rclcpp::TimerBase::SharedPtr system_state_timer_;
  rclcpp::TimerBase::SharedPtr healthcheck_timer_;
};

#endif  // MTMS_SIMULATOR__MTMS_SIMULATOR_HPP_
