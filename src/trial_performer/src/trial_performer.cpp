#include "headers/trial_performer.h"

#include "std_msgs/msg/empty.hpp"

using namespace std::chrono_literals;

const std::string HEARTBEAT_TOPIC = "/mtms/trial_performer/heartbeat";
constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

static constexpr float VOLTAGE_RELATIVE_ERROR_TOLERANCE = 0.03f;

TrialPerformerNode::TrialPerformerNode(const rclcpp::NodeOptions &options)
    : Node("trial_performer_node", options),
      callback_group(this->create_callback_group(rclcpp::CallbackGroupType::Reentrant)),
      reentrant_callback_group(this->create_callback_group(rclcpp::CallbackGroupType::Reentrant)),
      id_counter(0) {

  initialize_services();
  initialize_service_clients();
  initialize_subscribers();
  initialize_publishers();

  auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
  this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
    heartbeat_publisher->publish(std_msgs::msg::Empty());
  });
}

void TrialPerformerNode::initialize_services() {
  perform_trial_service = this->create_service<mtms_trial_interfaces::srv::PerformTrial>(
      "/mtms/trial/perform",
      std::bind(
          &TrialPerformerNode::handle_perform_trial,
          this,
          std::placeholders::_1,
          std::placeholders::_2),
      rmw_qos_profile_services_default,
      callback_group);

  prepare_trial_service = this->create_service<mtms_trial_interfaces::srv::PrepareTrial>(
      "/mtms/trial/prepare",
      std::bind(
          &TrialPerformerNode::handle_prepare_trial,
          this,
          std::placeholders::_1,
          std::placeholders::_2),
      rmw_qos_profile_services_default,
      callback_group);
}

void TrialPerformerNode::initialize_service_clients() {
  get_default_waveform_client = this->create_client<mtms_targeting_interfaces::srv::GetDefaultWaveform>(
      "/mtms/waveforms/get_default",
      rclcpp::QoS(rclcpp::ServicesQoS()),
      reentrant_callback_group);
  while (!get_default_waveform_client->wait_for_service(2s)) {
    RCLCPP_INFO(get_logger(), "Service /mtms/waveforms/get_default not available, waiting...");
  }

  get_multipulse_waveforms_client = this->create_client<mtms_targeting_interfaces::srv::GetMultipulseWaveforms>("/mtms/waveforms/get_multipulse_waveforms");
  while (!get_multipulse_waveforms_client->wait_for_service(2s)) {
    RCLCPP_INFO(get_logger(), "Service /mtms/waveforms/get_multipulse_waveforms not available, waiting...");
  }

  request_events_client = this->create_client<mtms_device_interfaces::srv::RequestEvents>("/mtms/device/events/request");
  while (!request_events_client->wait_for_service(2s)) {
    RCLCPP_INFO(get_logger(), "Service /mtms/device/events/request not available, waiting...");
  }

  /* Service client for setting voltages. */
  set_voltages_client = this->create_client<mtms_trial_interfaces::srv::SetVoltages>("/mtms/trial/set_voltages");
  while (!set_voltages_client->wait_for_service(2s)) {
    RCLCPP_INFO(get_logger(), "Service /mtms/trial/set_voltages not available, waiting...");
  }
}

void TrialPerformerNode::initialize_subscribers() {
  system_statesubscriber = this->create_subscription<mtms_device_interfaces::msg::SystemState>(
      "/mtms/device/system_state",
      1,
      std::bind(&TrialPerformerNode::handle_system_state, this, std::placeholders::_1));

  session_subscriber = this->create_subscription<mtms_system_interfaces::msg::Session>(
      "/mtms/device/session",
      1,
      std::bind(&TrialPerformerNode::handle_session, this, std::placeholders::_1));

  pulse_feedback_subscriber = this->create_subscription<mtms_event_interfaces::msg::PulseFeedback>(
      "/mtms/device/events/feedback/pulse", 10,
      std::bind(&TrialPerformerNode::update_pulse_feedback, this, std::placeholders::_1));

  trigger_out_feedback_subscriber = this->create_subscription<mtms_event_interfaces::msg::TriggerOutFeedback>(
      "/mtms/device/events/feedback/trigger_out", 10,
      std::bind(&TrialPerformerNode::update_trigger_out_feedback, this, std::placeholders::_1));
}

void TrialPerformerNode::initialize_publishers() {
  create_marker_publisher = this->create_publisher<mtms_neuronavigation_interfaces::msg::CreateMarker>("/neuronavigation/create_marker", 10);
}

/* Subscriber callbacks */

void TrialPerformerNode::handle_system_state(const mtms_device_interfaces::msg::SystemState::SharedPtr msg) {
  system_state = msg;
}

void TrialPerformerNode::handle_session(const mtms_system_interfaces::msg::Session::SharedPtr msg) {
  session = msg;
}

void TrialPerformerNode::update_pulse_feedback(const mtms_event_interfaces::msg::PulseFeedback::SharedPtr msg) {
  pulse_feedback[msg->id] = msg;

  auto error_code = msg->error.value;
  auto execution_time = msg->execution_time;

  /* If the event fails, the execution time will be 0; to avoid confusion, do not log it in that case. */
  if (error_code == 0) {
    RCLCPP_INFO(get_logger(), "Pulse event %d finished with error code %d at %.3f s", msg->id, msg->error.value, msg->execution_time);
  } else {
    RCLCPP_WARN(get_logger(), "Pulse event %d finished with error code %d", msg->id, msg->error.value);
  }
}

void TrialPerformerNode::update_trigger_out_feedback(const mtms_event_interfaces::msg::TriggerOutFeedback::SharedPtr msg) {
  trigger_out_feedback[msg->id] = msg;

  auto error_code = msg->error.value;
  auto execution_time = msg->execution_time;

  /* If the event fails, the execution time will be 0; to avoid confusion, do not log it in that case. */
  if (error_code == 0) {
    RCLCPP_INFO(get_logger(), "Trigger out event %d finished with error code %d at %.3f s", msg->id, msg->error.value, msg->execution_time);
  } else {
    RCLCPP_WARN(get_logger(), "Trigger out event %d finished with error code %d", msg->id, msg->error.value);
  }
}

/* Using publishers */

void TrialPerformerNode::create_marker(const mtms_trial_interfaces::msg::Trial &trial) {
  RCLCPP_INFO(this->get_logger(), "Creating marker...");

  auto msg = std::make_shared<mtms_neuronavigation_interfaces::msg::CreateMarker>();
  msg->targets = trial.targets;
  create_marker_publisher->publish(*msg);
}

/* Helpers */

bool TrialPerformerNode::check_trial_feasible() {
  if (!is_device_started()) {
    RCLCPP_WARN(this->get_logger(), "Device not started.");
    return false;
  }
  if (!is_session_started()) {
    RCLCPP_WARN(this->get_logger(), "Session not started.");
    return false;
  }
  return true;
}

bool TrialPerformerNode::is_device_started() const {
  return system_state && system_state->device_state.value == mtms_device_interfaces::msg::DeviceState::OPERATIONAL;
}

bool TrialPerformerNode::is_session_started() const {
  return session && session->state == mtms_system_interfaces::msg::Session::STARTED;
}

double TrialPerformerNode::get_current_time() const {
  if (session) {
    return session->time;
  }
  return 0.0;
}

int TrialPerformerNode::get_next_id() {
  return ++id_counter;
}

std::vector<uint16_t> TrialPerformerNode::get_actual_voltages() const {
  std::vector<uint16_t> voltages;
  for (uint8_t i = 0; i < NUM_OF_CHANNELS; ++i) {
    voltages.push_back(system_state->channel_states[i].voltage);
  }
  return voltages;
}

/* ROS message creation */

std::pair<std::vector<mtms_event_interfaces::msg::Pulse>, std::vector<uint16_t>> TrialPerformerNode::create_pulses(const std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> &waveforms, const mtms_trial_interfaces::msg::Trial &trial, double start_time) {
  std::vector<uint16_t> pulse_ids;
  std::vector<mtms_event_interfaces::msg::Pulse> pulses;

  for (uint8_t i = 0; i < trial.targets.size(); ++i) {
    auto waveforms_for_coil_set = waveforms[i];
    for (uint8_t channel = 0; channel < NUM_OF_CHANNELS; ++channel) {
      uint16_t id = get_next_id();
      auto waveform = waveforms_for_coil_set.waveforms[channel];
      auto pulse = create_pulse(id, channel, waveform, start_time + trial.pulse_times_since_trial_start[i], mtms_event_interfaces::msg::ExecutionCondition::TIMED);
      pulse_ids.push_back(id);
      pulses.push_back(pulse);
    }
  }

  return {pulses, pulse_ids};
}

mtms_event_interfaces::msg::Pulse TrialPerformerNode::create_pulse(uint16_t id, uint8_t channel, const mtms_waveform_interfaces::msg::Waveform &waveform, double time, uint8_t execution_condition) {
  mtms_event_interfaces::msg::EventInfo event_info;
  event_info.id = id;
  event_info.execution_condition.value = execution_condition;
  event_info.execution_time = time;

  mtms_event_interfaces::msg::Pulse pulse;
  pulse.event_info = event_info;
  pulse.channel = channel;
  pulse.waveform = waveform;

  return pulse;
}

std::pair<std::vector<mtms_event_interfaces::msg::TriggerOut>, std::vector<uint16_t>> TrialPerformerNode::create_trigger_outs(const mtms_trial_interfaces::msg::Trial &trial, double pulse_time) {
  std::vector<uint16_t> trigger_out_ids;
  std::vector<mtms_event_interfaces::msg::TriggerOut> trigger_outs;

  const auto n = std::min(trial.trigger_enabled.size(), trial.trigger_delay.size());
  for (size_t i = 0; i < n; ++i) {
    if (trial.trigger_enabled[i]) {
      int id = get_next_id();
      auto trigger_out = create_trigger_out(
          id,
          pulse_time + trial.trigger_delay[i],
          mtms_event_interfaces::msg::ExecutionCondition::TIMED,
          static_cast<uint8_t>(i + 1));
      trigger_out_ids.push_back(id);
      trigger_outs.push_back(trigger_out);
    }
  }

  return {trigger_outs, trigger_out_ids};
}

mtms_event_interfaces::msg::TriggerOut TrialPerformerNode::create_trigger_out(uint16_t id, double time, uint8_t execution_condition, uint8_t port) {
  mtms_event_interfaces::msg::EventInfo event_info;
  event_info.id = id;
  event_info.execution_condition.value = execution_condition;
  event_info.execution_time = time;

  mtms_event_interfaces::msg::TriggerOut trigger_out;
  trigger_out.event_info = event_info;
  trigger_out.port = port;
  trigger_out.duration_us = TRIGGER_DURATION_US;

  return trigger_out;
}

/* Events */

bool TrialPerformerNode::wait_for_events_to_finish(const std::vector<uint16_t> &pulse_ids, const std::vector<uint16_t> &trigger_out_ids) {
  while (true) {
    bool all_pulses_finished = std::all_of(pulse_ids.begin(), pulse_ids.end(), [this](uint16_t id) {
      return pulse_feedback.find(id) != pulse_feedback.end();
    });

    bool all_triggers_finished = std::all_of(trigger_out_ids.begin(), trigger_out_ids.end(), [this](uint16_t id) {
      return trigger_out_feedback.find(id) != trigger_out_feedback.end();
    });

    if (all_pulses_finished && all_triggers_finished) {
      break;
    }

    std::this_thread::sleep_for(1ms);
  }

  /* Check that all error codes are zero. */
  bool all_error_codes_zero = true;
  for (const auto &entry : pulse_feedback) {
    if (entry.second->error.value != 0) {
      all_error_codes_zero = false;
      break;
    }
  }
  for (const auto &entry : trigger_out_feedback) {
    if (entry.second->error.value != 0) {
      all_error_codes_zero = false;
      break;
    }
  }

  /* Clear feedback. */
  pulse_feedback.clear();
  trigger_out_feedback.clear();

  return all_error_codes_zero;
}

/* Logging */

void TrialPerformerNode::log_trial(const mtms_trial_interfaces::msg::Trial &trial) {
  RCLCPP_INFO(this->get_logger(), "Trial:");
  RCLCPP_INFO(this->get_logger(), "  Start time: %.3f s", trial.start_time);
  RCLCPP_INFO(this->get_logger(), "  Number of targets: %zu", trial.targets.size());
  RCLCPP_INFO(this->get_logger(), "  Number of pulses: %zu", trial.pulse_times_since_trial_start.size());
  for (size_t i = 0; i < trial.pulse_times_since_trial_start.size(); ++i) {
    RCLCPP_INFO(this->get_logger(), "    Pulse %zu time: %.3f s", i, trial.pulse_times_since_trial_start[i]);
  }
  for (size_t i = 0; i < trial.trigger_enabled.size(); ++i) {
    RCLCPP_INFO(this->get_logger(), "    Trigger %zu enabled: %s", i, trial.trigger_enabled[i] ? "true" : "false");
    RCLCPP_INFO(this->get_logger(), "    Trigger %zu delay: %.3f s", i, trial.trigger_delay[i]);
  }
}

void TrialPerformerNode::tic() {
  start_time = std::chrono::high_resolution_clock::now();
}

void TrialPerformerNode::toc(const std::string &prefix) {
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

  double duration_ms = duration.count() / 1000.0;

  RCLCPP_INFO(this->get_logger(), "%s: %.1f ms", prefix.c_str(), duration_ms);
}

/* Service requests */

std::pair<std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> TrialPerformerNode::get_approximated_waveforms(
    const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets,
    const std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> &target_waveforms) {

  auto request = std::make_shared<mtms_targeting_interfaces::srv::GetMultipulseWaveforms::Request>();
  request->targets = targets;
  request->target_waveforms = target_waveforms;

  auto future = get_multipulse_waveforms_client->async_send_request(request);

  /* Wait for the response. */
  auto future_status = future.wait_for(60s);
  if (future_status != std::future_status::ready) {
    throw std::runtime_error("Call to GetMultipulseWaveforms service timed out");
  }
  auto response = future.get();

  if (!response->success) {
    throw std::runtime_error("Call to GetMultipulseWaveforms service failed");
  }

  std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> approximated_waveforms(
      response->approximated_waveforms.begin(), response->approximated_waveforms.end());

  return {response->initial_voltages, approximated_waveforms};
}

mtms_waveform_interfaces::msg::Waveform TrialPerformerNode::get_default_waveform(uint8_t channel) {
  auto request = std::make_shared<mtms_targeting_interfaces::srv::GetDefaultWaveform::Request>();
  request->channel = channel;

  auto future = get_default_waveform_client->async_send_request(request);

  /* Wait for the response. */
  auto future_status = future.wait_for(10s);
  if (future_status != std::future_status::ready) {
    throw std::runtime_error("Call to GetDefaultWaveform service timed out");
  }
  auto response = future.get();

  auto waveform = response->waveform;
  return waveform;
}

void TrialPerformerNode::request_events(const std::vector<mtms_event_interfaces::msg::Pulse> &pulses, const std::vector<mtms_event_interfaces::msg::TriggerOut> &trigger_outs) {
  auto request = std::make_shared<mtms_device_interfaces::srv::RequestEvents::Request>();
  request->pulses = pulses;
  request->trigger_outs = trigger_outs;

  auto future = request_events_client->async_send_request(request);

  /* Wait for the response. */
  auto future_status = future.wait_for(10s);
  if (future_status != std::future_status::ready) {
    throw std::runtime_error("Call to RequestEvents service timed out");
  }
  auto response = future.get();

  if (!response->success) {
    throw std::runtime_error("Call to RequestEvents service failed");
  }
}

/* Action clients */

bool TrialPerformerNode::set_voltages(const std::vector<uint16_t> &voltages) {
  auto request = std::make_shared<mtms_trial_interfaces::srv::SetVoltages::Request>();
  request->voltages = voltages;

  auto future = set_voltages_client->async_send_request(request);

  /* Wait for the response. */
  if (future.wait_for(10s) != std::future_status::ready) {
    RCLCPP_ERROR(this->get_logger(), "Call to SetVoltages service timed out");
    return false;
  }

  auto response = future.get();
  if (!response->success) {
    RCLCPP_ERROR(this->get_logger(), "Call to SetVoltages service failed");
    return false;
  }

  return true;
}

void TrialPerformerNode::handle_perform_trial(
    const std::shared_ptr<mtms_trial_interfaces::srv::PerformTrial::Request> request,
    std::shared_ptr<mtms_trial_interfaces::srv::PerformTrial::Response> response) {

  tic();

  RCLCPP_INFO(this->get_logger(), "Received perform trial request, executing...");

  /* Log trial details. */
  log_trial(request->trial);

  /* Check feasibility. */
  if (!check_trial_feasible()) {
    RCLCPP_WARN(this->get_logger(), "Trial not feasible.");
    response->success = false;
    return;
  }

  const auto &trial = request->trial;

  auto targets = trial.targets;
  /* Always get desired voltages and waveforms (also warms up the cache). */
  auto [desired_voltages, waveforms] = get_desired_voltages_and_waveforms(targets);

  /* Voltages must already be within margin; fail if they are not. */
  auto actual_voltages = get_actual_voltages();

  if (!are_voltages_within_margin(desired_voltages)) {
    RCLCPP_ERROR(this->get_logger(), "Voltages not within margin; call prepare_trial first.");
    response->success = false;
    return;
  }

  /* Check if the trial can be performed at the desired start time. */
  const double_t earliest_start_time = get_current_time() + TRIAL_TIME_MARGINAL_S;
  if (earliest_start_time > trial.start_time) {
    RCLCPP_ERROR(this->get_logger(),
      "Trial desired start time (%.3f s) is in the past (earliest possible start: %.3f s).",
      trial.start_time, earliest_start_time);
    response->success = false;
    return;
  }

  /* Create and send pulses and trigger outs. */
  auto [pulses, pulse_ids] = create_pulses(waveforms, trial, trial.start_time);
  auto [trigger_outs, trigger_out_ids] = create_trigger_outs(trial, trial.start_time);

  /* Request events. */
  request_events(pulses, trigger_outs);

  /* For troubleshooting purposes, log the time passed after requesting events. */
  toc("Time passed after requesting events");

  /* Wait for events to finish. */
  bool success = wait_for_events_to_finish(pulse_ids, trigger_out_ids);

  auto voltages_after_trial = get_actual_voltages();

  /* If trial was successful, create a marker in neuronavigation. */
  if (success) {
    create_marker(trial);
  }

  /* Log trial success. */
  RCLCPP_INFO(this->get_logger(), "Trial completed %s.", success ? "successfully" : "with errors");

  response->success = success;
}

void TrialPerformerNode::handle_prepare_trial(
    const std::shared_ptr<mtms_trial_interfaces::srv::PrepareTrial::Request> request,
    std::shared_ptr<mtms_trial_interfaces::srv::PrepareTrial::Response> response) {
  RCLCPP_INFO(this->get_logger(), "Received prepare trial request, executing...");

  /* Log trial details. */
  log_trial(request->trial);

  if (!check_trial_feasible()) {
    RCLCPP_WARN(this->get_logger(), "Trial not feasible.");
    response->success = false;
    return;
  }

  auto targets = request->trial.targets;

  auto [desired_voltages, _] = get_desired_voltages_and_waveforms(targets);

  bool success = set_voltages(desired_voltages);
  RCLCPP_INFO(this->get_logger(), "Prepare trial completed %s.", success ? "successfully" : "with errors");
  response->success = success;
}

std::pair<std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> TrialPerformerNode::get_desired_voltages_and_waveforms(const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets) {
  std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> target_waveforms;
  for (uint8_t i = 0; i < targets.size(); ++i) {
    mtms_waveform_interfaces::msg::WaveformsForCoilSet waveforms;
    for (uint8_t channel = 0; channel < NUM_OF_CHANNELS; ++channel) {
      waveforms.waveforms.push_back(get_default_waveform(channel));
    }
    target_waveforms.push_back(waveforms);
  }
  return get_approximated_waveforms(targets, target_waveforms);
}

bool TrialPerformerNode::are_voltages_within_margin(const std::vector<uint16_t> &desired_voltages) const {
  auto actual_voltages = get_actual_voltages();

  for (uint8_t i = 0; i < actual_voltages.size(); ++i) {
    auto relative_error = std::abs(actual_voltages[i] - desired_voltages[i]) / static_cast<float>(actual_voltages[i]);
    if (relative_error > VOLTAGE_RELATIVE_ERROR_TOLERANCE &&
        std::abs(actual_voltages[i] - desired_voltages[i]) > ABSOLUTE_VOLTAGE_ERROR_THRESHOLD_FOR_PRECHARGING) {
      RCLCPP_WARN(this->get_logger(), "Voltage out of margin on channel %d (relative error: %.0f%%, absolute error: %d V).",
        i,
        100 * relative_error,
        std::abs(actual_voltages[i] - desired_voltages[i]));
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  /* TODO: Should this be set to real-time priority, as it's a part of the RT pathway to the mTMS device? */

  auto trial_performer_node = std::make_shared<TrialPerformerNode>();

  /* Use a small, explicit MultiThreadedExecutor thread pool so we can process incoming
   * service responses even while service handlers are blocked waiting on other service
   * futures, without over-parallelizing and causing contention/jitter. */
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 4);
  executor.add_node(trial_performer_node);
  executor.spin();

  rclcpp::shutdown();

  return 0;
}
