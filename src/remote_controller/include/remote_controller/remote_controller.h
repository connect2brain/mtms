#ifndef remote_controller__remote_controller_HPP_
#define remote_controller__remote_controller_HPP_

#include <mutex>
#include <optional>
#include <cstdint>

#include "rclcpp/rclcpp.hpp"

#include "mtms_system_interfaces/msg/timebase_mapping.hpp"
#include "shared_stimulation_interfaces/msg/targeted_pulses.hpp"

#include "mtms_trial_interfaces/msg/trial.hpp"
#include "mtms_trial_interfaces/srv/perform_trial.hpp"

#include "mtms_targeting_interfaces/msg/electric_target.hpp"

class RemoteController : public rclcpp::Node {
public:
  explicit RemoteController(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
  // ROS callbacks
  void eeg_to_mtms_callback(const mtms_system_interfaces::msg::TimebaseMapping::SharedPtr msg);
  void targeted_pulses_callback(const shared_stimulation_interfaces::msg::TargetedPulses::SharedPtr msg);

  // Helpers
  bool build_trial_from_message(
    const shared_stimulation_interfaces::msg::TargetedPulses & msg,
    const mtms_system_interfaces::msg::TimebaseMapping & mapping,
    mtms_trial_interfaces::msg::Trial & trial_out) const;

  static int8_t clamp_displacement_mm_to_int8(double mm);
  static int16_t wrap_rotation_deg_to_int16(double degrees);
  static uint8_t clamp_intensity_to_uint8(double intensity_v_m);

  // Subscriptions
  rclcpp::Subscription<mtms_system_interfaces::msg::TimebaseMapping>::SharedPtr eeg_to_mtms_subscriber;
  rclcpp::Subscription<shared_stimulation_interfaces::msg::TargetedPulses>::SharedPtr targeted_pulses_subscriber;

  // Service client
  rclcpp::Client<mtms_trial_interfaces::srv::PerformTrial>::SharedPtr perform_trial_client;

  // Latest timebase mapping (EEG device time -> mTMS session time).
  std::optional<mtms_system_interfaces::msg::TimebaseMapping> latest_eeg_to_mtms;

  // Prevent overlapping trials.
  std::mutex trial_ongoing_mutex;
  bool trial_ongoing{false};

  // Constants aligned with `targeting` validation.
  static constexpr int8_t MAX_ABSOLUTE_DISPLACEMENT_MM = 18;
  static constexpr uint8_t INTENSITY_LIMIT_V_M = 135;
  static constexpr uint16_t MAX_ROTATION_ANGLE_DEG = 359;
  static constexpr double DEFAULT_VOLTAGE_TOLERANCE_PROPORTION_FOR_PRECHARGING = 0.03;
};

#endif  // remote_controller__remote_controller_HPP_
