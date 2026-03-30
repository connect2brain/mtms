#ifndef TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_
#define TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_

#include <optional>

#include "rclcpp/rclcpp.hpp"

#include "mtms_eeg_interfaces/msg/sample.hpp"
#include "mtms_system_interfaces/msg/session.hpp"
#include "mtms_system_interfaces/msg/timebase_mapping.hpp"

class TimebaseCalibrator : public rclcpp::Node {
public:
  TimebaseCalibrator();

private:
  void eeg_callback(const mtms_eeg_interfaces::msg::Sample::SharedPtr msg);
  void session_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg);

  /* Session tracking. */
  bool session_active = false;

  /* Sync trigger counting: increments by 1 each time trigger_b is true.
     Corresponds to mTMS session time in whole seconds. */
  int sync_trigger_count = -1;

  /* EEG device timestamp of the most recent sync trigger (trigger_b),
     regardless of whether a session is active. Used for the staleness
     check when a session starts. */
  std::optional<double> last_sync_trigger_eeg_timestamp;

  /* Latest EEG device timestamp seen (any sample), used to judge staleness. */
  double latest_eeg_timestamp = 0.0;

  rclcpp::Subscription<mtms_eeg_interfaces::msg::Sample>::SharedPtr eeg_subscription;
  rclcpp::Subscription<mtms_system_interfaces::msg::Session>::SharedPtr session_subscription;

  rclcpp::Publisher<mtms_system_interfaces::msg::TimebaseMapping>::SharedPtr mapping_publisher;

  rclcpp::TimerBase::SharedPtr heartbeat_timer;
};

#endif  // TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_
