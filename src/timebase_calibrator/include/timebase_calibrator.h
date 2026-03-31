#ifndef TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_
#define TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_

#include "rclcpp/rclcpp.hpp"

#include "mtms_eeg_interfaces/msg/sample.hpp"
#include "mtms_system_interfaces/msg/component_health.hpp"
#include "mtms_system_interfaces/msg/session.hpp"
#include "mtms_system_interfaces/msg/timebase_mapping.hpp"

class TimebaseCalibrator : public rclcpp::Node {
public:
  TimebaseCalibrator();

private:
  void eeg_callback(const mtms_eeg_interfaces::msg::Sample::SharedPtr msg);
  void session_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg);
  void publish_health();

  /* Session tracking. */
  bool session_active = false;

  /* Window-based sync trigger tracking. */
  int    expected_trigger_number = 1;
  int    consecutive_miss_count  = 0;
  bool   in_error_state          = false;
  double window_start            = 0.0;
  double window_end              = 0.0;

  /* Latest EEG device timestamp seen (any sample), used to set the initial window. */
  double latest_eeg_timestamp = 0.0;

  rclcpp::Subscription<mtms_eeg_interfaces::msg::Sample>::SharedPtr eeg_subscription;
  rclcpp::Subscription<mtms_system_interfaces::msg::Session>::SharedPtr session_subscription;

  rclcpp::Publisher<mtms_system_interfaces::msg::TimebaseMapping>::SharedPtr mapping_publisher;
  rclcpp::Publisher<mtms_system_interfaces::msg::ComponentHealth>::SharedPtr health_publisher;

  rclcpp::TimerBase::SharedPtr heartbeat_timer;
  rclcpp::TimerBase::SharedPtr health_timer;
};

#endif  // TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_
