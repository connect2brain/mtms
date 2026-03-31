#ifndef TRIAL_PERFORMER_H
#define TRIAL_PERFORMER_H

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <tuple>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "mtms_trial_interfaces/msg/trial.hpp"
#include "mtms_trial_interfaces/msg/trial_state.hpp"
#include "mtms_trial_interfaces/srv/cache_target_list.hpp"
#include "mtms_trial_interfaces/srv/set_voltages.hpp"
#include "mtms_device_interfaces/msg/device_state.hpp"
#include "mtms_device_interfaces/msg/system_state.hpp"
#include "mtms_device_interfaces/srv/request_events.hpp"
#include "mtms_neuronavigation_interfaces/msg/create_marker.hpp"
#include "mtms_system_interfaces/msg/session.hpp"
#include "mtms_targeting_interfaces/srv/get_default_waveform.hpp"
#include "mtms_targeting_interfaces/srv/get_multipulse_waveforms.hpp"
#include "mtms_event_interfaces/msg/event_info.hpp"
#include "mtms_event_interfaces/msg/pulse.hpp"
#include "mtms_event_interfaces/msg/pulse_feedback.hpp"
#include "mtms_event_interfaces/msg/trigger_out.hpp"
#include "mtms_event_interfaces/msg/trigger_out_feedback.hpp"

/* NOTE: If this value changes, update all places that assume fixed maximum voltage. */
static constexpr uint16_t FIXED_DESIRED_VOLTAGE = 1500;

static constexpr int NUM_OF_CHANNELS = 5;
static constexpr int TRIGGER_DURATION_US = 1000;
static constexpr float ABSOLUTE_VOLTAGE_ERROR_TOLERANCE = 10.0;

/* ──────────────────────────────────────────────────────────────────────
 * SharedState — data accessed by both the hot-path and helper nodes.
 * All access is protected by the corresponding mutex.
 * ────────────────────────────────────────────────────────────────────── */
struct SharedState {
  /* Device / session state (written by HelperNode subscribers, read by HotPathNode) */
  mtms_device_interfaces::msg::SystemState::SharedPtr system_state;
  mutable std::mutex state_mutex;

  mtms_system_interfaces::msg::Session::SharedPtr session;
  mutable std::mutex session_mutex;

  /* Event feedback (written by HelperNode subscribers, read by HotPathNode) */
  std::unordered_map<uint16_t, mtms_event_interfaces::msg::PulseFeedback::SharedPtr> pulse_feedback;
  std::unordered_map<uint16_t, mtms_event_interfaces::msg::TriggerOutFeedback::SharedPtr> trigger_out_feedback;
  mutable std::mutex feedback_mutex;

  /* Waveform cache (written by HelperNode, read by HotPathNode) */
  std::unordered_map<std::string, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> waveform_cache;
  mutable std::mutex cache_mutex;

  /* Fixed desired voltages */
  std::vector<uint16_t> fixed_desired_voltages;

  /* Monotonic event-ID counter */
  std::atomic<uint16_t> id_counter{0};

  /* Busy flag shared across both nodes */
  std::atomic<bool> busy{false};

  /* Helpers that operate on shared data */
  bool is_device_started() const;
  bool is_session_started() const;
  double get_current_time() const;
  uint16_t get_next_id();
  std::vector<uint16_t> get_actual_voltages() const;
  bool is_ready_for_trial(bool verbose, const rclcpp::Logger &logger) const;
  std::string targets_to_cache_key(const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets) const;
};

struct BusyGuard {
  std::atomic<bool> &flag;
  explicit BusyGuard(std::atomic<bool> &f) : flag(f) {}
  ~BusyGuard() { flag = false; }
};

/* ──────────────────────────────────────────────────────────────────────
 * HotPathNode — owns the perform_trial service, the request_events
 * client, and the marker publisher.
 * ────────────────────────────────────────────────────────────────────── */
class HotPathNode : public rclcpp::Node {
public:
  explicit HotPathNode(std::shared_ptr<SharedState> shared_state,
                       const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

private:
  std::shared_ptr<SharedState> state;

  rclcpp::CallbackGroup::SharedPtr callback_group;

  /* Subscription */
  rclcpp::Subscription<mtms_trial_interfaces::msg::Trial>::SharedPtr perform_trial_subscriber;

  /* Service client */
  rclcpp::Client<mtms_device_interfaces::srv::RequestEvents>::SharedPtr request_events_client;

  /* Publishers */
  rclcpp::Publisher<mtms_neuronavigation_interfaces::msg::CreateMarker>::SharedPtr create_marker_publisher;
  rclcpp::Publisher<mtms_trial_interfaces::msg::TrialState>::SharedPtr trial_state_publisher;

  /* Topic callback */
  void handle_perform_trial(const mtms_trial_interfaces::msg::Trial::SharedPtr msg);

  /* ROS message creation */
  std::pair<std::vector<mtms_event_interfaces::msg::Pulse>, std::vector<uint16_t>> create_pulses(
      const std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> &waveforms,
      const mtms_trial_interfaces::msg::Trial &trial, double start_time);

  mtms_event_interfaces::msg::Pulse create_pulse(
      uint16_t id, uint8_t channel, const mtms_waveform_interfaces::msg::Waveform &waveform,
      double time, uint8_t execution_condition);

  std::pair<std::vector<mtms_event_interfaces::msg::TriggerOut>, std::vector<uint16_t>> create_trigger_outs(
      const mtms_trial_interfaces::msg::Trial &trial, double pulse_time);

  mtms_event_interfaces::msg::TriggerOut create_trigger_out(
      uint16_t id, double time, uint8_t execution_condition, uint8_t port);


  /* Events */
  bool wait_for_events_to_finish(const std::vector<uint16_t> &pulse_ids,
                                 const std::vector<uint16_t> &trigger_out_ids);

  /* Publisher helpers */
  void create_marker(const mtms_trial_interfaces::msg::Trial &trial);

  /* Logging */
  void log_trial(const mtms_trial_interfaces::msg::Trial &trial);

  /* Timing */
  std::chrono::time_point<std::chrono::system_clock> start_time;
  void tic();
  void toc(const std::string &prefix);
};

/* ──────────────────────────────────────────────────────────────────────
 * HelperNode — owns cache_target_list and prepare_trial services,
 * system-state / session subscriptions, event feedback subscriptions,
 * waveform service clients, voltage setter, heartbeat, and
 * trial-readiness publisher.
 * ────────────────────────────────────────────────────────────────────── */
class HelperNode : public rclcpp::Node {
public:
  explicit HelperNode(std::shared_ptr<SharedState> shared_state,
                      const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

private:
  std::shared_ptr<SharedState> state;

  rclcpp::CallbackGroup::SharedPtr callback_group;
  rclcpp::CallbackGroup::SharedPtr reentrant_callback_group;

  /* Services */
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr prepare_trial_service;
  rclcpp::Service<mtms_trial_interfaces::srv::CacheTargetList>::SharedPtr cache_target_list_service;

  /* Service clients */
  rclcpp::Client<mtms_trial_interfaces::srv::SetVoltages>::SharedPtr set_voltages_client;
  rclcpp::Client<mtms_targeting_interfaces::srv::GetDefaultWaveform>::SharedPtr get_default_waveform_client;
  rclcpp::Client<mtms_targeting_interfaces::srv::GetMultipulseWaveforms>::SharedPtr get_multipulse_waveforms_client;

  /* Subscribers */
  rclcpp::Subscription<mtms_device_interfaces::msg::SystemState>::SharedPtr system_state_subscriber;
  rclcpp::Subscription<mtms_system_interfaces::msg::Session>::SharedPtr session_subscriber;
  rclcpp::Subscription<mtms_event_interfaces::msg::PulseFeedback>::SharedPtr pulse_feedback_subscriber;
  rclcpp::Subscription<mtms_event_interfaces::msg::TriggerOutFeedback>::SharedPtr trigger_out_feedback_subscriber;

  /* Publishers */
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr trial_readiness_publisher;

  /* Timer */
  rclcpp::TimerBase::SharedPtr heartbeat_timer;

  /* Subscriber callbacks */
  void handle_system_state(const mtms_device_interfaces::msg::SystemState::SharedPtr msg);
  void handle_session(const mtms_system_interfaces::msg::Session::SharedPtr msg);
  void update_pulse_feedback(const mtms_event_interfaces::msg::PulseFeedback::SharedPtr msg);
  void update_trigger_out_feedback(const mtms_event_interfaces::msg::TriggerOutFeedback::SharedPtr msg);

  /* Service handlers */
  void handle_cache_target_list(
      const std::shared_ptr<mtms_trial_interfaces::srv::CacheTargetList::Request> request,
      std::shared_ptr<mtms_trial_interfaces::srv::CacheTargetList::Response> response);

  void handle_prepare_trial(
      const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
      std::shared_ptr<std_srvs::srv::Trigger::Response> response);

  /* Service calls */
  std::tuple<bool, std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> get_approximated_waveforms(
      const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets,
      const std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> &target_waveforms);

  mtms_waveform_interfaces::msg::Waveform get_default_waveform(uint8_t channel);
  bool set_voltages(const std::vector<uint16_t> &voltages);

  std::tuple<bool, std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> get_desired_voltages_and_waveforms(
      const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets);

  /* Logging */
  void log_voltages(const std::vector<uint16_t> &voltages, const std::string &prefix);

  /* Timing */
  std::chrono::time_point<std::chrono::system_clock> start_time;
  void tic();
  void toc(const std::string &prefix);
};

#endif // TRIAL_PERFORMER_H
