#ifndef TRIAL_PERFORMER_H
#define TRIAL_PERFORMER_H

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "mtms_trial_interfaces/msg/trial.hpp"
#include "mtms_trial_interfaces/srv/perform_trial.hpp"
#include "mtms_trial_interfaces/srv/set_voltages.hpp"
#include "mtms_device_interfaces/msg/device_state.hpp"
#include "mtms_device_interfaces/msg/system_state.hpp"
#include "mtms_device_interfaces/srv/request_events.hpp"
#include "mtms_neuronavigation_interfaces/msg/create_marker.hpp"
#include "mtms_system_interfaces/msg/session.hpp"
#include "mtms_targeting_interfaces/srv/get_target_voltages.hpp"
#include "mtms_targeting_interfaces/srv/reverse_polarity.hpp"
#include "mtms_targeting_interfaces/srv/get_default_waveform.hpp"
#include "mtms_targeting_interfaces/srv/get_multipulse_waveforms.hpp"
#include "mtms_event_interfaces/msg/event_info.hpp"
#include "mtms_event_interfaces/msg/pulse.hpp"
#include "mtms_event_interfaces/msg/pulse_feedback.hpp"
#include "mtms_event_interfaces/msg/trigger_out.hpp"
#include "mtms_event_interfaces/msg/trigger_out_feedback.hpp"

class TrialPerformerNode : public rclcpp::Node {
public:
  static constexpr int NUM_OF_CHANNELS = 5;
  static constexpr int TRIGGER_DURATION_US = 1000;
  static constexpr float TRIAL_TIME_MARGINAL_S = 0.1;
  static constexpr float ABSOLUTE_VOLTAGE_ERROR_THRESHOLD_FOR_PRECHARGING = 5.0;

  explicit TrialPerformerNode(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

private:
  rclcpp::CallbackGroup::SharedPtr callback_group;
  rclcpp::CallbackGroup::SharedPtr reentrant_callback_group;
  rclcpp::Service<mtms_trial_interfaces::srv::PerformTrial>::SharedPtr perform_trial_service;
  rclcpp::Client<mtms_trial_interfaces::srv::SetVoltages>::SharedPtr set_voltages_client;
  rclcpp::Client<mtms_targeting_interfaces::srv::GetTargetVoltages>::SharedPtr targeting_client;
  rclcpp::Client<mtms_targeting_interfaces::srv::ReversePolarity>::SharedPtr reverse_polarity_client;
  rclcpp::Client<mtms_targeting_interfaces::srv::GetDefaultWaveform>::SharedPtr get_default_waveform_client;
  rclcpp::Client<mtms_targeting_interfaces::srv::GetMultipulseWaveforms>::SharedPtr get_multipulse_waveforms_client;
  rclcpp::Client<mtms_device_interfaces::srv::RequestEvents>::SharedPtr request_events_client;
  rclcpp::Subscription<mtms_device_interfaces::msg::SystemState>::SharedPtr system_statesubscriber;
  rclcpp::Subscription<mtms_system_interfaces::msg::Session>::SharedPtr session_subscriber;
  rclcpp::Subscription<mtms_event_interfaces::msg::PulseFeedback>::SharedPtr pulse_feedback_subscriber;
  rclcpp::Subscription<mtms_event_interfaces::msg::TriggerOutFeedback>::SharedPtr trigger_out_feedback_subscriber;
  rclcpp::Publisher<mtms_neuronavigation_interfaces::msg::CreateMarker>::SharedPtr create_marker_publisher;

  mtms_device_interfaces::msg::SystemState::SharedPtr system_state;
  mtms_system_interfaces::msg::Session::SharedPtr session;
  std::unordered_map<uint16_t, mtms_event_interfaces::msg::PulseFeedback::SharedPtr> pulse_feedback;
  std::unordered_map<uint16_t, mtms_event_interfaces::msg::TriggerOutFeedback::SharedPtr> trigger_out_feedback;
  uint16_t id_counter;

  void initialize_services();
  void initialize_service_clients();
  void initialize_subscribers();
  void initialize_publishers();

  void handle_system_state(const mtms_device_interfaces::msg::SystemState::SharedPtr msg);
  void handle_session(const mtms_system_interfaces::msg::Session::SharedPtr msg);
  void update_pulse_feedback(const mtms_event_interfaces::msg::PulseFeedback::SharedPtr msg);
  void update_trigger_out_feedback(const mtms_event_interfaces::msg::TriggerOutFeedback::SharedPtr msg);

  /* Helpers */
  bool check_trial_feasible();
  bool is_device_started() const;
  bool is_session_started() const;
  double get_current_time() const;
  int get_next_id();
  std::vector<uint16_t> get_actual_voltages() const;

  /* Events */
  bool wait_for_events_to_finish(const std::vector<uint16_t> &pulse_ids, const std::vector<uint16_t> &trigger_out_ids);

  /* ROS message creation */
  std::pair<std::vector<mtms_event_interfaces::msg::Pulse>, std::vector<uint16_t>> create_pulses(
      const std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> &waveforms, const mtms_trial_interfaces::msg::Trial &trial, double start_time);

  mtms_event_interfaces::msg::Pulse create_pulse(uint16_t id, uint8_t channel, const mtms_waveform_interfaces::msg::Waveform &waveform, double time, uint8_t execution_condition);

  std::pair<std::vector<mtms_event_interfaces::msg::TriggerOut>, std::vector<uint16_t>> create_trigger_outs(
      const mtms_trial_interfaces::msg::Trial &trial, double pulse_time);

  mtms_event_interfaces::msg::TriggerOut create_trigger_out(uint16_t id, double time, uint8_t execution_condition, uint8_t port);

  /* Service calls */
  std::pair<std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> get_approximated_waveforms(
      const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets,
      const std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> &target_waveforms);

  std::pair<std::vector<double_t>, std::vector<bool>> get_target_voltages(
      const mtms_targeting_interfaces::msg::ElectricTarget &target);

  mtms_waveform_interfaces::msg::Waveform get_default_waveform(uint8_t channel);
  mtms_waveform_interfaces::msg::Waveform reverse_polarity(const mtms_waveform_interfaces::msg::Waveform &waveform);
  void request_events(const std::vector<mtms_event_interfaces::msg::Pulse> &pulses, const std::vector<mtms_event_interfaces::msg::TriggerOut> &trigger_outs);
  bool set_voltages(const std::vector<uint16_t> &voltages);
  bool set_voltages_if_needed(const std::vector<uint16_t> &desired_voltages, float voltage_tolerance_proportion_for_precharging);

  /* Service handlers */
  void handle_perform_trial(
      const std::shared_ptr<mtms_trial_interfaces::srv::PerformTrial::Request> request,
      std::shared_ptr<mtms_trial_interfaces::srv::PerformTrial::Response> response);

  /* Publishers */
  void create_marker(const mtms_trial_interfaces::msg::Trial &trial);

  /* Logging */
  void log_trial(const mtms_trial_interfaces::msg::Trial &trial);
  void log_voltages(const std::vector<uint16_t> &voltages, const std::string &prefix);

  /* Timing */
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
  void tic();
  void toc(const std::string &prefix);

  bool perform_trial(const mtms_trial_interfaces::msg::Trial &trial);

  std::pair<std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> get_non_approximated_waveforms(
      const mtms_targeting_interfaces::msg::ElectricTarget &target, const mtms_waveform_interfaces::msg::WaveformsForCoilSet &target_waveforms);

  std::pair<std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> get_desired_voltages_and_waveforms(
      const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets, const bool use_pwm_approximation);
};

#endif // TRIAL_PERFORMER_H
