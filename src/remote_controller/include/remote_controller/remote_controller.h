#ifndef remote_controller__remote_controller_HPP_
#define remote_controller__remote_controller_HPP_

#include <optional>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/bool.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "mtms_system_interfaces/msg/timebase_mapping.hpp"
#include "shared_stimulation_interfaces/msg/targeted_pulses.hpp"

#include "mtms_trial_interfaces/msg/target_list.hpp"
#include "mtms_trial_interfaces/msg/trial.hpp"
#include "mtms_trial_interfaces/msg/remote_controller_state.hpp"
#include "mtms_trial_interfaces/srv/perform_trial.hpp"
#include "mtms_trial_interfaces/srv/cache_target_list.hpp"
#include "mtms_trial_interfaces/srv/start_remote_controller.hpp"

#include "mtms_targeting_interfaces/msg/electric_target.hpp"
#include "mtms_system_interfaces/msg/component_health.hpp"
#include "mtms_system_interfaces/msg/session.hpp"
#include "mtms_system_interfaces/srv/start_session.hpp"
#include "mtms_system_interfaces/srv/stop_session.hpp"

class RemoteController : public rclcpp::Node {
public:
  explicit RemoteController(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
  // ROS service handlers
  void start_service_handler(
    const std::shared_ptr<mtms_trial_interfaces::srv::StartRemoteController::Request> request,
    std::shared_ptr<mtms_trial_interfaces::srv::StartRemoteController::Response> response);
  void stop_service_handler(
    [[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response);

  // ROS subscribers and publishers
  void timebase_mapping_callback(const mtms_system_interfaces::msg::TimebaseMapping::SharedPtr msg);
  void targeted_pulses_callback(const shared_stimulation_interfaces::msg::TargetedPulses::SharedPtr msg);
  void trial_readiness_callback(const std_msgs::msg::Bool::SharedPtr msg);
  void session_state_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg);
  void publish_remote_controller_state();

  // Trial caching and validation
  void cache_target_lists_async(std::vector<mtms_trial_interfaces::msg::TargetList> target_lists);
  bool is_trial_target_list_compatible(const mtms_trial_interfaces::msg::Trial & trial);

  void set_state(uint8_t new_state);
  uint8_t get_state();
  void publish_health();

  void prepare_trial();

  // Helpers
  bool start_session();
  bool stop_session();
  bool build_trial_from_message(
    const shared_stimulation_interfaces::msg::TargetedPulses & msg,
    const mtms_system_interfaces::msg::TimebaseMapping & mapping,
    mtms_trial_interfaces::msg::Trial & trial_out) const;

  static int8_t clamp_displacement_mm_to_int8(double mm);
  static int16_t wrap_rotation_deg_to_int16(double degrees);
  static uint8_t clamp_intensity_to_uint8(double intensity_v_m);

  // Subscriptions
  rclcpp::Subscription<mtms_system_interfaces::msg::TimebaseMapping>::SharedPtr timebase_mapping_subscriber;
  rclcpp::Subscription<shared_stimulation_interfaces::msg::TargetedPulses>::SharedPtr targeted_pulses_subscriber;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr trial_readiness_subscriber;
  rclcpp::Subscription<mtms_system_interfaces::msg::Session>::SharedPtr session_subscriber;

  // Session state tracking for blocking start/stop.
  std::atomic<bool> is_session_started{false};

  // Service clients
  rclcpp::Client<mtms_trial_interfaces::srv::PerformTrial>::SharedPtr perform_trial_client;
  rclcpp::Client<mtms_trial_interfaces::srv::CacheTargetList>::SharedPtr cache_target_list_client;
  rclcpp::Client<mtms_system_interfaces::srv::StartSession>::SharedPtr start_session_client;
  rclcpp::Client<mtms_system_interfaces::srv::StopSession>::SharedPtr stop_session_client;
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr prepare_trial_client;

  // Start/stop gating.
  rclcpp::Service<mtms_trial_interfaces::srv::StartRemoteController>::SharedPtr start_service;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_service;
  rclcpp::Publisher<mtms_trial_interfaces::msg::RemoteControllerState>::SharedPtr state_publisher;
  rclcpp::Publisher<mtms_system_interfaces::msg::ComponentHealth>::SharedPtr health_publisher;
  rclcpp::TimerBase::SharedPtr health_timer;

  // Latest timebase mapping (EEG device time -> mTMS session time).
  std::optional<mtms_system_interfaces::msg::TimebaseMapping> latest_timebase_mapping;

  // Remote controller state machine.
  mtms_trial_interfaces::msg::RemoteControllerState state;
  std::mutex state_mutex;

  // Trial readiness state.
  std::atomic<bool> trial_readiness{false};
  std::atomic<bool> prepare_trial_ongoing{false};

  // Prevent overlapping trials.
  std::atomic<bool> trial_ongoing{false};

  // Health: true when the last TargetedPulses was rejected due to target list mismatch.
  std::atomic<bool> target_list_mismatch{false};

  // Stored target lists for validating trials.
  std::vector<std::vector<mtms_targeting_interfaces::msg::ElectricTarget>> stored_target_lists;

  // Constants aligned with `targeting` validation.
  static constexpr int8_t MAX_ABSOLUTE_DISPLACEMENT_MM = 18;
  static constexpr uint8_t INTENSITY_LIMIT_V_M = 135;
  static constexpr uint16_t MAX_ROTATION_ANGLE_DEG = 359;

  rclcpp::TimerBase::SharedPtr heartbeat_timer;
};

#endif  // remote_controller__remote_controller_HPP_
