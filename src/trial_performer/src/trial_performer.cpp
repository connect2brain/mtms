#include "headers/trial_performer.h"

#include <cassert>

#include "std_msgs/msg/empty.hpp"

#include "realtime_utils/utils.h"

using namespace std::chrono_literals;

const std::string HEARTBEAT_TOPIC = "/mtms/trial_performer/heartbeat";
constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

const std::string TRIAL_READINESS_TOPIC = "/mtms/trial/trial_readiness";

static constexpr std::chrono::seconds VOLTAGE_WAIT_TIMEOUT{20};
static constexpr std::chrono::seconds EVENT_FEEDBACK_TIMEOUT{10};

/* ══════════════════════════════════════════════════════════════════════
 * SharedState
 * ══════════════════════════════════════════════════════════════════════ */

bool SharedState::is_device_started() const {
  std::lock_guard<std::mutex> lock(state_mutex);
  return system_state && system_state->device_state.value == mtms_device_interfaces::msg::DeviceState::OPERATIONAL;
}

bool SharedState::is_session_started() const {
  std::lock_guard<std::mutex> lock(session_mutex);
  return session && session->state == mtms_system_interfaces::msg::Session::STARTED;
}

double SharedState::get_current_time() const {
  std::lock_guard<std::mutex> lock(session_mutex);
  if (session) {
    return session->time;
  }
  return 0.0;
}

uint16_t SharedState::get_next_id() {
  return ++id_counter;
}

std::vector<uint16_t> SharedState::get_actual_voltages() const {
  std::lock_guard<std::mutex> lock(state_mutex);
  std::vector<uint16_t> voltages;
  for (uint8_t i = 0; i < NUM_OF_CHANNELS; ++i) {
    voltages.push_back(system_state->channel_states[i].voltage);
  }
  return voltages;
}

bool SharedState::is_ready_for_trial(bool verbose, const rclcpp::Logger &logger) const {
  auto actual_voltages = get_actual_voltages();
  const auto &desired_voltages = fixed_desired_voltages;

  for (uint8_t i = 0; i < actual_voltages.size(); ++i) {
    if (std::abs(actual_voltages[i] - desired_voltages[i]) > ABSOLUTE_VOLTAGE_ERROR_TOLERANCE) {
      if (verbose) {
        RCLCPP_WARN(logger,
          "Voltage out of margin on channel %d (absolute error: %d V).",
          i,
          std::abs(actual_voltages[i] - desired_voltages[i]));
      }
      return false;
    }
  }
  return true;
}

std::string SharedState::targets_to_cache_key(const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets) const {
  std::ostringstream oss;
  for (const auto &t : targets) {
    oss << static_cast<int>(t.displacement_x) << ','
        << static_cast<int>(t.displacement_y) << ','
        << t.rotation_angle << ','
        << static_cast<int>(t.intensity) << ','
        << static_cast<int>(t.algorithm) << ';';
  }
  return oss.str();
}

/* ══════════════════════════════════════════════════════════════════════
 * HotPathNode
 * ══════════════════════════════════════════════════════════════════════ */

HotPathNode::HotPathNode(std::shared_ptr<SharedState> shared_state, const rclcpp::NodeOptions &options)
    : Node("trial_performer_hot_node", options),
      state(std::move(shared_state)),
      callback_group(this->create_callback_group(rclcpp::CallbackGroupType::Reentrant)) {

  /* Service */
  perform_trial_service = this->create_service<mtms_trial_interfaces::srv::PerformTrial>(
      "/mtms/trial/perform",
      std::bind(
          &HotPathNode::handle_perform_trial,
          this,
          std::placeholders::_1,
          std::placeholders::_2),
      rmw_qos_profile_services_default,
      callback_group);

  /* Service client */
  request_events_client = this->create_client<mtms_device_interfaces::srv::RequestEvents>("/mtms/device/events/request");
  while (!request_events_client->wait_for_service(2s)) {
    RCLCPP_INFO(get_logger(), "Service /mtms/device/events/request not available, waiting...");
  }

  /* Publishers */
  create_marker_publisher = this->create_publisher<mtms_neuronavigation_interfaces::msg::CreateMarker>("/neuronavigation/create_marker", 10);
}

/* ROS message creation */

std::pair<std::vector<mtms_event_interfaces::msg::Pulse>, std::vector<uint16_t>> HotPathNode::create_pulses(const std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> &waveforms, const mtms_trial_interfaces::msg::Trial &trial, double start_time) {
  std::vector<uint16_t> pulse_ids;
  std::vector<mtms_event_interfaces::msg::Pulse> pulses;

  for (uint8_t i = 0; i < trial.targets.size(); ++i) {
    auto waveforms_for_coil_set = waveforms[i];
    for (uint8_t channel = 0; channel < NUM_OF_CHANNELS; ++channel) {
      uint16_t id = state->get_next_id();
      auto waveform = waveforms_for_coil_set.waveforms[channel];
      auto pulse = create_pulse(id, channel, waveform, start_time + trial.pulse_times_since_trial_start[i], mtms_event_interfaces::msg::ExecutionCondition::TIMED);
      pulse_ids.push_back(id);
      pulses.push_back(pulse);
    }
  }

  return {pulses, pulse_ids};
}

mtms_event_interfaces::msg::Pulse HotPathNode::create_pulse(uint16_t id, uint8_t channel, const mtms_waveform_interfaces::msg::Waveform &waveform, double time, uint8_t execution_condition) {
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

std::pair<std::vector<mtms_event_interfaces::msg::TriggerOut>, std::vector<uint16_t>> HotPathNode::create_trigger_outs(const mtms_trial_interfaces::msg::Trial &trial, double pulse_time) {
  std::vector<uint16_t> trigger_out_ids;
  std::vector<mtms_event_interfaces::msg::TriggerOut> trigger_outs;

  const auto n = std::min(trial.trigger_enabled.size(), trial.trigger_delay.size());
  for (size_t i = 0; i < n; ++i) {
    if (trial.trigger_enabled[i]) {
      int id = state->get_next_id();
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

mtms_event_interfaces::msg::TriggerOut HotPathNode::create_trigger_out(uint16_t id, double time, uint8_t execution_condition, uint8_t port) {
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

/* Service calls */


/* Events */

bool HotPathNode::wait_for_events_to_finish(const std::vector<uint16_t> &pulse_ids, const std::vector<uint16_t> &trigger_out_ids) {
  auto wait_start = std::chrono::steady_clock::now();

  while (true) {
    {
      std::lock_guard<std::mutex> lock(state->feedback_mutex);
      bool all_pulses_finished = std::all_of(pulse_ids.begin(), pulse_ids.end(), [this](uint16_t id) {
        return state->pulse_feedback.find(id) != state->pulse_feedback.end();
      });

      bool all_triggers_finished = std::all_of(trigger_out_ids.begin(), trigger_out_ids.end(), [this](uint16_t id) {
        return state->trigger_out_feedback.find(id) != state->trigger_out_feedback.end();
      });

      if (all_pulses_finished && all_triggers_finished) {
        break;
      }
    }

    if (std::chrono::steady_clock::now() - wait_start > EVENT_FEEDBACK_TIMEOUT) {
      RCLCPP_ERROR(this->get_logger(), "Timed out waiting for event feedback; device may have dropped events.");
      std::lock_guard<std::mutex> lock(state->feedback_mutex);
      state->pulse_feedback.clear();
      state->trigger_out_feedback.clear();
      return false;
    }

    std::this_thread::sleep_for(1ms);
  }

  /* Check that all error codes are zero. */
  std::lock_guard<std::mutex> lock(state->feedback_mutex);
  bool all_error_codes_zero = true;
  for (const auto &entry : state->pulse_feedback) {
    if (entry.second->error.value != 0) {
      all_error_codes_zero = false;
      break;
    }
  }
  for (const auto &entry : state->trigger_out_feedback) {
    if (entry.second->error.value != 0) {
      all_error_codes_zero = false;
      break;
    }
  }

  /* Clear feedback. */
  state->pulse_feedback.clear();
  state->trigger_out_feedback.clear();

  return all_error_codes_zero;
}

/* Publisher helpers */

void HotPathNode::create_marker(const mtms_trial_interfaces::msg::Trial &trial) {
  RCLCPP_INFO(this->get_logger(), "Creating marker...");

  auto msg = std::make_shared<mtms_neuronavigation_interfaces::msg::CreateMarker>();
  msg->targets = trial.targets;
  create_marker_publisher->publish(*msg);
}

/* Service handler */

void HotPathNode::handle_perform_trial(
    const std::shared_ptr<mtms_trial_interfaces::srv::PerformTrial::Request> request,
    std::shared_ptr<mtms_trial_interfaces::srv::PerformTrial::Response> response) {

  const auto _t_start = std::chrono::system_clock::now();

  response->success = false;
  if (state->busy.exchange(true)) {
    RCLCPP_ERROR(this->get_logger(), "Perform trial request received while busy.");
    return;
  }
  BusyGuard busy_guard{state->busy};

  tic();

  /* Check that the device and session are started. */
  if (!state->is_device_started()) {
    RCLCPP_WARN(this->get_logger(), "Device not started.");
    return;
  }

  if (!state->is_session_started()) {
    RCLCPP_WARN(this->get_logger(), "Session not started.");
    return;
  }

  const auto &trial = request->trial;
  auto targets = trial.targets;

  /* Look up pre-cached waveforms; fail if the target list has not been cached. */
  std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> waveforms;
  {
    std::lock_guard<std::mutex> lock(state->cache_mutex);
    auto it = state->waveform_cache.find(state->targets_to_cache_key(targets));
    if (it == state->waveform_cache.end()) {
      RCLCPP_ERROR(this->get_logger(), "Perform trial failed: target list has not been cached; call cache_target_list first.");
      return;
    }
    waveforms = it->second;
  }

  /* Check if the trial is ready to be performed. */
  if (!state->is_ready_for_trial(true, get_logger())) {
    RCLCPP_ERROR(this->get_logger(), "Trial not ready to be performed; call prepare_trial first.");
    return;
  }

  /* Create and send pulses and trigger outs. */
  auto [pulses, pulse_ids] = create_pulses(waveforms, trial, trial.start_time);
  auto [trigger_outs, trigger_out_ids] = create_trigger_outs(trial, trial.start_time);

  /* Request events. */
  auto request_events_req = std::make_shared<mtms_device_interfaces::srv::RequestEvents::Request>();
  request_events_req->pulses = pulses;
  request_events_req->trigger_outs = trigger_outs;

  auto future = request_events_client->async_send_request(request_events_req);

  /* Spawn background thread to handle async event request and waiting. */
  std::thread([this, future = std::move(future),
               pulse_ids, trigger_out_ids, trial = request->trial, _t_start, response]() mutable {
    try {
      /* Wait for the request_events call to complete. */
      auto response_ptr = future.get();

      toc("Time passed after requesting events");

      RCLCPP_INFO(this->get_logger(), " ");
      RCLCPP_INFO(this->get_logger(), "Received perform trial request, executing...");

      log_trial(trial);

      if (!response_ptr || !response_ptr->success) {
        RCLCPP_ERROR(this->get_logger(), "Perform trial failed: failed to request events.");
        response->success = false;
        return;
      }

      /* Wait for events to finish. */
      bool success = wait_for_events_to_finish(pulse_ids, trigger_out_ids);
      response->success = success;

      /* If trial was successful, create a marker in neuronavigation. */
      if (success) {
        create_marker(trial);
      }

      RCLCPP_INFO(this->get_logger(), "Trial completed %s.", success ? "successfully" : "with errors");
      RCLCPP_INFO(this->get_logger(), " ");

      const auto _t_end = std::chrono::system_clock::now();
      const double _t_start_s = std::chrono::duration<double>(_t_start.time_since_epoch()).count();
      const double _t_end_s = std::chrono::duration<double>(_t_end.time_since_epoch()).count();
      const double _duration_ms = std::chrono::duration<double, std::milli>(_t_end - _t_start).count();
      RCLCPP_INFO(this->get_logger(), "handle_perform_trial: start=%.3f s, end=%.3f s, duration=%.1f ms", _t_start_s, _t_end_s, _duration_ms);
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "Perform trial failed: %s", e.what());
      response->success = false;
    }
  }).join();
}

/* Logging */

void HotPathNode::log_trial(const mtms_trial_interfaces::msg::Trial &trial) {
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

void HotPathNode::tic() {
  start_time = std::chrono::high_resolution_clock::now();
}

void HotPathNode::toc(const std::string &prefix) {
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

  double duration_ms = duration.count() / 1000.0;

  RCLCPP_INFO(this->get_logger(), "%s: %.1f ms", prefix.c_str(), duration_ms);
}

/* ══════════════════════════════════════════════════════════════════════
 * HelperNode
 * ══════════════════════════════════════════════════════════════════════ */

HelperNode::HelperNode(std::shared_ptr<SharedState> shared_state, const rclcpp::NodeOptions &options)
    : Node("trial_performer_helper_node", options),
      state(std::move(shared_state)),
      callback_group(this->create_callback_group(rclcpp::CallbackGroupType::Reentrant)),
      reentrant_callback_group(this->create_callback_group(rclcpp::CallbackGroupType::Reentrant)) {

  /* Services */
  cache_target_list_service = this->create_service<mtms_trial_interfaces::srv::CacheTargetList>(
      "/mtms/trial/cache",
      std::bind(
          &HelperNode::handle_cache_target_list,
          this,
          std::placeholders::_1,
          std::placeholders::_2),
      rmw_qos_profile_services_default,
      callback_group);

  prepare_trial_service = this->create_service<std_srvs::srv::Trigger>(
      "/mtms/trial/prepare",
      std::bind(
          &HelperNode::handle_prepare_trial,
          this,
          std::placeholders::_1,
          std::placeholders::_2),
      rmw_qos_profile_services_default,
      callback_group);

  /* Service clients */
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

  set_voltages_client = this->create_client<mtms_trial_interfaces::srv::SetVoltages>("/mtms/trial/set_voltages");
  while (!set_voltages_client->wait_for_service(2s)) {
    RCLCPP_INFO(get_logger(), "Service /mtms/trial/set_voltages not available, waiting...");
  }

  /* Subscribers */
  system_state_subscriber = this->create_subscription<mtms_device_interfaces::msg::SystemState>(
      "/mtms/device/system_state",
      1,
      std::bind(&HelperNode::handle_system_state, this, std::placeholders::_1));

  session_subscriber = this->create_subscription<mtms_system_interfaces::msg::Session>(
      "/mtms/device/session",
      1,
      std::bind(&HelperNode::handle_session, this, std::placeholders::_1));

  pulse_feedback_subscriber = this->create_subscription<mtms_event_interfaces::msg::PulseFeedback>(
      "/mtms/device/events/feedback/pulse", 10,
      std::bind(&HelperNode::update_pulse_feedback, this, std::placeholders::_1));

  trigger_out_feedback_subscriber = this->create_subscription<mtms_event_interfaces::msg::TriggerOutFeedback>(
      "/mtms/device/events/feedback/trigger_out", 10,
      std::bind(&HelperNode::update_trigger_out_feedback, this, std::placeholders::_1));

  /* Publishers */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);
  trial_readiness_publisher = this->create_publisher<std_msgs::msg::Bool>(TRIAL_READINESS_TOPIC, qos_persist_latest);

  /* Heartbeat */
  auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
  heartbeat_timer = this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
    heartbeat_publisher->publish(std_msgs::msg::Empty());
  });
}

/* Subscriber callbacks */

void HelperNode::handle_system_state(const mtms_device_interfaces::msg::SystemState::SharedPtr msg) {
  {
    std::lock_guard<std::mutex> lock(state->state_mutex);
    state->system_state = msg;
  }

  std_msgs::msg::Bool readiness_msg;
  readiness_msg.data = state->is_ready_for_trial(false, get_logger());
  trial_readiness_publisher->publish(readiness_msg);
}

void HelperNode::handle_session(const mtms_system_interfaces::msg::Session::SharedPtr msg) {
  std::lock_guard<std::mutex> lock(state->session_mutex);
  state->session = msg;
}

void HelperNode::update_pulse_feedback(const mtms_event_interfaces::msg::PulseFeedback::SharedPtr msg) {
  {
    std::lock_guard<std::mutex> lock(state->feedback_mutex);
    state->pulse_feedback[msg->id] = msg;
  }

  auto error_code = msg->error.value;

  /* If the event fails, the execution time will be 0; to avoid confusion, do not log it in that case. */
  if (error_code == 0) {
    RCLCPP_INFO(get_logger(), "Pulse event %d finished with error code %d at %.3f s", msg->id, msg->error.value, msg->execution_time);
  } else {
    RCLCPP_WARN(get_logger(), "Pulse event %d finished with error code %d", msg->id, msg->error.value);
  }
}

void HelperNode::update_trigger_out_feedback(const mtms_event_interfaces::msg::TriggerOutFeedback::SharedPtr msg) {
  {
    std::lock_guard<std::mutex> lock(state->feedback_mutex);
    state->trigger_out_feedback[msg->id] = msg;
  }

  auto error_code = msg->error.value;

  /* If the event fails, the execution time will be 0; to avoid confusion, do not log it in that case. */
  if (error_code == 0) {
    RCLCPP_INFO(get_logger(), "Trigger out event %d finished with error code %d at %.3f s", msg->id, msg->error.value, msg->execution_time);
  } else {
    RCLCPP_WARN(get_logger(), "Trigger out event %d finished with error code %d", msg->id, msg->error.value);
  }
}

/* Service handlers */

void HelperNode::handle_prepare_trial(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response) {

  (void)request;  // No request data; this call is used purely as a trigger.
  response->success = false;

  if (state->busy.exchange(true)) {
    RCLCPP_ERROR(this->get_logger(), "Prepare trial request received while busy.");
    return;
  }
  BusyGuard busy_guard{state->busy};

  RCLCPP_INFO(this->get_logger(), " ");
  RCLCPP_INFO(this->get_logger(), "Received prepare trial request, charging max voltages...");

  /* Check that the device and session are started. */
  if (!state->is_device_started()) {
    RCLCPP_WARN(this->get_logger(), "Device not started.");
    return;
  }

  if (!state->is_session_started()) {
    RCLCPP_WARN(this->get_logger(), "Session not started.");
    return;
  }

  /* Wait for voltage setter to finish and report final status. */
  if (!set_voltages(state->fixed_desired_voltages)) {
    RCLCPP_ERROR(this->get_logger(), "Failed to set voltages.");
    return;
  }

  response->success = true;
  RCLCPP_INFO(this->get_logger(), "Prepare trial completed successfully.");
  RCLCPP_INFO(this->get_logger(), " ");
}

void HelperNode::handle_cache_target_list(
    const std::shared_ptr<mtms_trial_interfaces::srv::CacheTargetList::Request> request,
    std::shared_ptr<mtms_trial_interfaces::srv::CacheTargetList::Response> response) {

  response->success = false;

  if (state->busy.exchange(true)) {
    RCLCPP_ERROR(this->get_logger(), "Cache target list request received while busy.");
    return;
  }
  BusyGuard busy_guard{state->busy};

  tic();

  RCLCPP_INFO(this->get_logger(), " ");
  RCLCPP_INFO(this->get_logger(), "Received cache target list request, warming cache...");

  const auto targets = request->targets;

  /* Warm the waveform cache by calling get_desired_voltages_and_waveforms(). */
  auto [approximation_success, desired_voltages, waveforms] = get_desired_voltages_and_waveforms(targets);
  if (!approximation_success) {
    RCLCPP_ERROR(this->get_logger(), "Cache trial warm-up failed: waveform approximation could not be performed.");
    return;
  }
  (void)desired_voltages;

  {
    std::lock_guard<std::mutex> lock(state->cache_mutex);
    state->waveform_cache[state->targets_to_cache_key(targets)] = waveforms;
  }

  response->success = true;

  toc("Time passed while warming cache");
}

/* Service calls */

std::tuple<bool, std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> HelperNode::get_approximated_waveforms(
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
    return {false, {}, {}};
  }

  std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> approximated_waveforms(
      response->approximated_waveforms.begin(), response->approximated_waveforms.end());

  return {true, response->initial_voltages, approximated_waveforms};
}

mtms_waveform_interfaces::msg::Waveform HelperNode::get_default_waveform(uint8_t channel) {
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

bool HelperNode::set_voltages(const std::vector<uint16_t> &voltages) {
  auto request = std::make_shared<mtms_trial_interfaces::srv::SetVoltages::Request>();
  request->voltages = voltages;

  auto future = set_voltages_client->async_send_request(request);

  auto future_status = future.wait_for(VOLTAGE_WAIT_TIMEOUT);
  if (future_status != std::future_status::ready) {
    RCLCPP_ERROR(this->get_logger(), "Timed out waiting for /mtms/trial/set_voltages response.");
    return false;
  }

  auto response = future.get();
  if (!response->success) {
    RCLCPP_ERROR(this->get_logger(), "Voltage setter returned failure.");
    return false;
  }

  return true;
}

std::tuple<bool, std::vector<uint16_t>, std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet>> HelperNode::get_desired_voltages_and_waveforms(const std::vector<mtms_targeting_interfaces::msg::ElectricTarget> &targets) {
  std::vector<mtms_waveform_interfaces::msg::WaveformsForCoilSet> target_waveforms;
  for (uint8_t i = 0; i < targets.size(); ++i) {
    mtms_waveform_interfaces::msg::WaveformsForCoilSet waveforms;
    for (uint8_t channel = 0; channel < NUM_OF_CHANNELS; ++channel) {
      waveforms.waveforms.push_back(get_default_waveform(channel));
    }
    target_waveforms.push_back(waveforms);
  }
  auto [approximation_success, desired_voltages, approximated_waveforms] = get_approximated_waveforms(targets, target_waveforms);
  if (!approximation_success) {
    return {false, {}, {}};
  }

  /* XXX: Have this check just to ensure the PWM approximation is made with the same desired voltage (1500 V) assumed by trial performer. */
  assert(desired_voltages == state->fixed_desired_voltages &&
      "Desired voltages from get_approximated_waveforms do not match FIXED_DESIRED_VOLTAGE");

  return {true, desired_voltages, approximated_waveforms};
}

/* Logging */

void HelperNode::log_voltages(const std::vector<uint16_t> &voltages, const std::string &prefix) {
  RCLCPP_INFO(this->get_logger(), "%s:", prefix.c_str());
  for (uint8_t i = 0; i < voltages.size(); ++i) {
    RCLCPP_INFO(this->get_logger(), "  Channel %d: %d V", i, voltages[i]);
  }
}

void HelperNode::tic() {
  start_time = std::chrono::high_resolution_clock::now();
}

void HelperNode::toc(const std::string &prefix) {
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

  double duration_ms = duration.count() / 1000.0;

  RCLCPP_INFO(this->get_logger(), "%s: %.1f ms", prefix.c_str(), duration_ms);
}

/* ══════════════════════════════════════════════════════════════════════
 * main
 * ══════════════════════════════════════════════════════════════════════ */

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto logger = rclcpp::get_logger("trial_performer");

  realtime_utils::MemoryConfig mem_config;
  mem_config.enable_memory_optimization = true;
  mem_config.preallocate_size = 10 * 1024 * 1024; // 10 MB

  realtime_utils::SchedulingConfig sched_config;
  sched_config.enable_scheduling_optimization = true;
  sched_config.scheduling_policy = SCHED_RR;
  sched_config.priority_level = realtime_utils::PriorityLevel::HIGHEST_REALTIME;

  try {
    realtime_utils::initialize_scheduling(sched_config, logger);
    realtime_utils::initialize_memory(mem_config, logger);
  } catch (const std::exception& e) {
    RCLCPP_FATAL(logger, "Initialization failed: %s", e.what());
    return -1;
  }

  auto shared_state = std::make_shared<SharedState>();
  shared_state->fixed_desired_voltages.assign(NUM_OF_CHANNELS, FIXED_DESIRED_VOLTAGE);

  auto hot_node = std::make_shared<HotPathNode>(shared_state);
  auto helper_node = std::make_shared<HelperNode>(shared_state);

  /* Hot-path executor is single-threaded for determinism. Event feedback callbacks
   * run on the helper executor, so blocking on wait_for_events_to_finish is safe. */
  rclcpp::executors::SingleThreadedExecutor hot_executor;
  rclcpp::executors::MultiThreadedExecutor helper_executor(rclcpp::ExecutorOptions(), 4);

  hot_executor.add_node(hot_node);
  helper_executor.add_node(helper_node);

  std::thread helper_thread([&helper_executor]() {
    helper_executor.spin();
  });

  hot_executor.spin();
  helper_thread.join();

  rclcpp::shutdown();

  return 0;
}
