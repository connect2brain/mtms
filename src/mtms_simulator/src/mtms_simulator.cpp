#include "mtms_simulator/mtms_simulator.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <cstdint>
#include <memory>
#include <thread>
#include <tuple>

#include "mtms_simulator/neurone_trigger_client.h"

#include "rclcpp/executors/single_threaded_executor.hpp"

#include "event_interfaces/msg/charge_error.hpp"
#include "event_interfaces/msg/discharge_error.hpp"
#include "event_interfaces/msg/trigger_out_error.hpp"
#include "mtms_device_interfaces/msg/channel_state.hpp"
#include "waveform_interfaces/msg/waveform_phase.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

MTMSSimulator::MTMSSimulator()
: Node("mtms_simulator")
{
  this->declare_parameter<int64_t>("channels", 5);
  this->declare_parameter<int64_t>("max_voltage", 1500);
  this->declare_parameter<int64_t>("charge_rate", 1500);

  num_of_channels_ = static_cast<size_t>(this->get_parameter("channels").as_int());
  max_voltage_ = static_cast<uint16_t>(this->get_parameter("max_voltage").as_int());
  charge_rate_ = static_cast<double>(this->get_parameter("charge_rate").as_int());

  RCLCPP_INFO(this->get_logger(), "Number of channels: %zu", num_of_channels_);

  send_settings_service_ = this->create_service<mtms_device_interfaces::srv::SendSettings>(
    "/mtms/device/send_settings", std::bind(&MTMSSimulator::send_settings_handler, this, _1, _2));
  start_device_service_ = this->create_service<std_srvs::srv::Trigger>(
    "/mtms/device/start", std::bind(&MTMSSimulator::start_device_handler, this, _1, _2));
  stop_device_service_ = this->create_service<std_srvs::srv::Trigger>(
    "/mtms/device/stop", std::bind(&MTMSSimulator::stop_device_handler, this, _1, _2));
  start_session_service_ = this->create_service<system_interfaces::srv::StartSession>(
    "/mtms/device/session/start", std::bind(&MTMSSimulator::start_session_handler, this, _1, _2));
  stop_session_service_ = this->create_service<system_interfaces::srv::StopSession>(
    "/mtms/device/session/stop", std::bind(&MTMSSimulator::stop_session_handler, this, _1, _2));
  request_events_service_ = this->create_service<mtms_device_interfaces::srv::RequestEvents>(
    "/mtms/device/events/request", std::bind(&MTMSSimulator::request_events_handler, this, _1, _2));
  trigger_events_service_ = this->create_service<std_srvs::srv::Trigger>(
    "/mtms/device/events/trigger", std::bind(&MTMSSimulator::trigger_events_handler, this, _1, _2));

  pulse_feedback_publisher_ = this->create_publisher<event_interfaces::msg::PulseFeedback>(
    "/mtms/device/events/feedback/pulse", 10);
  charge_feedback_publisher_ = this->create_publisher<event_interfaces::msg::ChargeFeedback>(
    "/mtms/device/events/feedback/charge", 10);
  discharge_feedback_publisher_ = this->create_publisher<event_interfaces::msg::DischargeFeedback>(
    "/mtms/device/events/feedback/discharge", 10);
  trigger_out_feedback_publisher_ = this->create_publisher<event_interfaces::msg::TriggerOutFeedback>(
    "/mtms/device/events/feedback/trigger_out", 10);

  auto state_qos = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  allow_stimulation_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
    "/mtms/stimulation/allowed", state_qos, std::bind(&MTMSSimulator::allow_stimulation_callback, this, _1));
  allow_trigger_out_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
    "/mtms/trigger_out/allowed", state_qos, std::bind(&MTMSSimulator::allow_trigger_out_callback, this, _1));

  const auto session_period = std::chrono::milliseconds(SESSION_PUBLISHING_INTERVAL_MS);
  const auto system_state_period = std::chrono::milliseconds(SYSTEM_STATE_PUBLISHING_INTERVAL_MS);
  const auto healthcheck_period = std::chrono::milliseconds(HEALTHCHECK_PUBLISHING_INTERVAL_MS);

  auto session_qos = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
  const auto session_lifetime =
    session_period + std::chrono::milliseconds(SESSION_PUBLISHING_INTERVAL_TOLERANCE_MS);
  session_qos.deadline(rclcpp::Duration::from_seconds(
    std::chrono::duration<double>(session_lifetime).count()));
  session_qos.lifespan(rclcpp::Duration::from_seconds(
    std::chrono::duration<double>(session_lifetime).count()));
  session_publisher_ = this->create_publisher<system_interfaces::msg::Session>(
    "/mtms/device/session", session_qos);

  auto system_state_qos = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);
  const auto system_state_lifetime =
    system_state_period + std::chrono::milliseconds(SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE_MS);
  system_state_qos.deadline(rclcpp::Duration::from_seconds(
    std::chrono::duration<double>(system_state_lifetime).count()));
  system_state_qos.lifespan(rclcpp::Duration::from_seconds(
    std::chrono::duration<double>(system_state_lifetime).count()));
  system_state_publisher_ = this->create_publisher<mtms_device_interfaces::msg::SystemState>(
    "/mtms/device/system_state", system_state_qos);

  healthcheck_publisher_ = this->create_publisher<system_interfaces::msg::Healthcheck>(
    "/mtms/device/healthcheck", 10);

  system_state_ = mtms_device_interfaces::msg::SystemState();
  system_state_.channel_states = std::vector<mtms_device_interfaces::msg::ChannelState>(num_of_channels_);

  channels_.reserve(num_of_channels_);
  for (size_t i = 0; i < num_of_channels_; ++i) {
    channels_.emplace_back(
      charge_rate_, CAPACITANCE, TIME_CONSTANT,
      PULSE_VOLTAGE_DROP_PROPORTION, max_voltage_,
      this->get_logger());
  }

  settings_ = mtms_device_interfaces::msg::Settings();
  settings_.maximum_number_of_pulse_pieces = DEFAULT_MAXIMUM_NUMBER_OF_PULSE_PIECES;
  settings_.maximum_rising_falling_difference_ticks = DEFAULT_MAXIMUM_RISING_FALLING_DIFFERENCE_TICKS;
  settings_.maximum_pulse_duration_ticks = DEFAULT_MAXIMUM_PULSE_DURATION_TICKS;
  settings_.pulses_in_maximum_pulses_per_time = DEFAULT_PULSES_IN_MAXIMUM_PULSES_PER_TIME;
  settings_.time_in_maximum_pulses_per_time_ms = DEFAULT_TIME_IN_MAXIMUM_PULSES_PER_TIME_MS;

  RCLCPP_INFO(
    this->get_logger(),
    "Using simulator default settings for pulse validation; these are reasonable defaults "
    "and may not exactly match mTMS hardware limits.");
  log_settings(settings_);

  // Connect to NeurOne trigger server in the background and keep retrying until it is reachable.
  neurone_trigger_client_ = std::make_unique<NeurOneTriggerClient>(this->get_logger());

  session_timer_ = this->create_wall_timer(
    session_period, std::bind(&MTMSSimulator::publish_session, this));
  system_state_timer_ = this->create_wall_timer(
    system_state_period, std::bind(&MTMSSimulator::publish_system_state, this));
  healthcheck_timer_ = this->create_wall_timer(
    healthcheck_period, std::bind(&MTMSSimulator::publish_healthcheck, this));

  event_worker_ = std::thread(&MTMSSimulator::event_worker_loop, this);
}

MTMSSimulator::~MTMSSimulator()
{
  {
    std::lock_guard<std::mutex> lock(event_queue_mutex_);
    stop_event_worker_ = true;
  }
  event_queue_cv_.notify_one();
  if (event_worker_.joinable()) {
    event_worker_.join();
  }
}

void MTMSSimulator::log_settings(const mtms_device_interfaces::msg::Settings & settings)
{
  RCLCPP_INFO(this->get_logger(), "Simulator settings:");
  RCLCPP_INFO(
    this->get_logger(), "  maximum number of pulse pieces: %u",
    settings.maximum_number_of_pulse_pieces);
  RCLCPP_INFO(
    this->get_logger(), "  maximum rising-falling difference (ticks): %u",
    settings.maximum_rising_falling_difference_ticks);
  RCLCPP_INFO(
    this->get_logger(), "  maximum pulse duration (ticks): %u",
    settings.maximum_pulse_duration_ticks);
  RCLCPP_INFO(
    this->get_logger(), "  maximum pulses per unit time, pulses: %u",
    settings.pulses_in_maximum_pulses_per_time);
  RCLCPP_INFO(
    this->get_logger(), "  maximum pulses per unit time, unit time (ms): %u",
    settings.time_in_maximum_pulses_per_time_ms);
}

void MTMSSimulator::allow_stimulation_callback(const std_msgs::msg::Bool::SharedPtr msg)
{
  RCLCPP_INFO(this->get_logger(), "Allow stimulation set to %s", msg->data ? "true" : "false");
  allow_stimulation_ = msg->data;
}

void MTMSSimulator::allow_trigger_out_callback(const std_msgs::msg::Bool::SharedPtr msg)
{
  RCLCPP_INFO(this->get_logger(), "Allow trigger out set to %s", msg->data ? "true" : "false");
  allow_trigger_out_ = msg->data;
}

void MTMSSimulator::send_settings_handler(
  const std::shared_ptr<mtms_device_interfaces::srv::SendSettings::Request> request,
  std::shared_ptr<mtms_device_interfaces::srv::SendSettings::Response> response)
{
  log_settings(request->settings);
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    settings_ = request->settings;
    for (auto & channel : channels_) {
      channel.settings = settings_;
    }
  }
  response->success = true;
}

void MTMSSimulator::start_device_handler(
  [[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  system_state_.device_state.value = mtms_device_interfaces::msg::DeviceState::OPERATIONAL;
  RCLCPP_INFO(this->get_logger(), "Device started");
  response->success = true;
}

void MTMSSimulator::stop_device_handler(
  [[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  system_state_.device_state.value = mtms_device_interfaces::msg::DeviceState::NOT_OPERATIONAL;
  RCLCPP_INFO(this->get_logger(), "Device stopped");
  response->success = true;
}

void MTMSSimulator::start_session_handler(
  [[maybe_unused]] const std::shared_ptr<system_interfaces::srv::StartSession::Request> request,
  std::shared_ptr<system_interfaces::srv::StartSession::Response> response)
{
  if (system_state_.device_state.value != mtms_device_interfaces::msg::DeviceState::OPERATIONAL) {
    RCLCPP_WARN(this->get_logger(), "Device not started. Can't start session");
    response->success = false;
    return;
  }

  session_state_value_ = system_interfaces::msg::Session::STARTED;
  session_start_time_ = this->get_clock()->now().seconds();
  RCLCPP_INFO(this->get_logger(), "Session started");
  response->success = true;
}

void MTMSSimulator::stop_session_handler(
  [[maybe_unused]] const std::shared_ptr<system_interfaces::srv::StopSession::Request> request,
  std::shared_ptr<system_interfaces::srv::StopSession::Response> response)
{
  session_state_value_ = system_interfaces::msg::Session::STOPPED;
  RCLCPP_INFO(this->get_logger(), "Session stopped");
  response->success = true;
}

void MTMSSimulator::request_events_handler(
  const std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Request> request,
  std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Response> response)
{
  queue_event_batch(request->pulses, request->charges, request->discharges, request->trigger_outs);
  response->success = true;
}

void MTMSSimulator::trigger_events_handler(
  [[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  RCLCPP_WARN(
    this->get_logger(),
    "Execution condition WAIT_FOR_TRIGGER is not supported in simulator.");
  response->success = true;
}

bool MTMSSimulator::session_not_started() const
{
  return session_state_value_.load() != system_interfaces::msg::Session::STARTED;
}

bool MTMSSimulator::validate_charge_or_discharge(
  const uint8_t channel,
  const uint16_t target_voltage,
  uint8_t & error_value) const
{
  if (channel >= system_state_.channel_states.size()) {
    RCLCPP_ERROR(
      this->get_logger(),
      "Trying to use invalid channel %u, configured channel count is %zu",
      channel, num_of_channels_);
    error_value = event_interfaces::msg::ChargeError::INVALID_CHANNEL;
    return false;
  }
  if (target_voltage >= max_voltage_) {
    RCLCPP_ERROR(
      this->get_logger(),
      "Too high voltage. Requested %u, maximum supported is %u",
      target_voltage, max_voltage_);
    error_value = event_interfaces::msg::ChargeError::INVALID_VOLTAGE;
    return false;
  }

  error_value = event_interfaces::msg::ChargeError::NO_ERROR;
  return true;
}

void MTMSSimulator::wait_for_execution_condition(
  const event_interfaces::msg::ExecutionCondition & execution_condition,
  const double execution_time) const
{
  switch (execution_condition.value) {
    case event_interfaces::msg::ExecutionCondition::TIMED:
    {
      const double wait =
        execution_time - (this->get_clock()->now().seconds() - session_start_time_.load());
      if (wait > 0.0) {
        std::this_thread::sleep_for(std::chrono::duration<double>(wait));
      }
      break;
    }
    case event_interfaces::msg::ExecutionCondition::WAIT_FOR_TRIGGER:
      RCLCPP_WARN(
        this->get_logger(),
        "Execution condition WAIT_FOR_TRIGGER not supported. Doing nothing.");
      break;
    case event_interfaces::msg::ExecutionCondition::IMMEDIATE:
      break;
    default:
      RCLCPP_WARN(
        this->get_logger(),
        "Invalid execution condition: %u, assuming IMMEDIATE",
        execution_condition.value);
      break;
  }
}

std::tuple<uint32_t, uint32_t, uint32_t> MTMSSimulator::calculate_waveform_durations(
  const waveform_interfaces::msg::Waveform & waveform) const
{
  uint32_t total_duration = 0;
  uint32_t rising_duration = 0;
  uint32_t falling_duration = 0;

  for (const auto & piece : waveform.pieces) {
    const uint32_t duration = piece.duration_in_ticks;
    total_duration += duration;
    if (piece.waveform_phase.value == waveform_interfaces::msg::WaveformPhase::RISING) {
      rising_duration += duration;
    } else if (piece.waveform_phase.value == waveform_interfaces::msg::WaveformPhase::FALLING) {
      falling_duration += duration;
    }
  }

  return {total_duration, rising_duration, falling_duration};
}

event_interfaces::msg::PulseError MTMSSimulator::validate_pulse(const event_interfaces::msg::Pulse & message) const
{
  event_interfaces::msg::PulseError error;
  error.value = event_interfaces::msg::PulseError::NO_ERROR;

  if (!allow_stimulation_.load()) {
    RCLCPP_WARN(this->get_logger(), "Stimulation not allowed, skipping pulse.");
    error.value = event_interfaces::msg::PulseError::NOT_ALLOWED;
    return error;
  }

  std::lock_guard<std::mutex> lock(state_mutex_);

  if (message.channel >= system_state_.channel_states.size()) {
    RCLCPP_WARN(this->get_logger(), "Invalid channel index: %u", message.channel);
    error.value = event_interfaces::msg::PulseError::INVALID_CHANNEL;
    return error;
  }

  const auto execution_condition = message.event_info.execution_condition.value;
  if (
    execution_condition != event_interfaces::msg::ExecutionCondition::IMMEDIATE &&
    execution_condition != event_interfaces::msg::ExecutionCondition::TIMED &&
    execution_condition != event_interfaces::msg::ExecutionCondition::WAIT_FOR_TRIGGER)
  {
    error.value = event_interfaces::msg::PulseError::INVALID_EXECUTION_CONDITION;
    return error;
  }

  const auto & channel = channels_[message.channel];
  if (channel.is_charging) {
    error.value = event_interfaces::msg::PulseError::OVERLAPPING_WITH_CHARGING;
    return error;
  }
  if (channel.is_discharging) {
    error.value = event_interfaces::msg::PulseError::OVERLAPPING_WITH_DISCHARGING;
    return error;
  }

  auto [total_duration, rising_duration, falling_duration] =
    calculate_waveform_durations(message.waveform);

  if (total_duration > settings_.maximum_pulse_duration_ticks) {
    RCLCPP_WARN(
      this->get_logger(),
      "Pulse duration invalid: total=%u ticks exceeds max=%u ticks",
      total_duration, settings_.maximum_pulse_duration_ticks);
    error.value = event_interfaces::msg::PulseError::INVALID_DURATIONS;
    return error;
  }

  const uint32_t rise_fall_diff =
    static_cast<uint32_t>(std::abs(
    static_cast<int64_t>(rising_duration) - static_cast<int64_t>(falling_duration)));
  if (rise_fall_diff > settings_.maximum_rising_falling_difference_ticks) {
    RCLCPP_WARN(
      this->get_logger(),
      "Pulse duration invalid: rising=%u ticks, falling=%u ticks, diff=%u ticks exceeds max_diff=%u ticks",
      rising_duration, falling_duration, rise_fall_diff,
      settings_.maximum_rising_falling_difference_ticks);
    error.value = event_interfaces::msg::PulseError::INVALID_DURATIONS;
    return error;
  }

  return error;
}

void MTMSSimulator::process_charge(const event_interfaces::msg::Charge & message)
{
  RCLCPP_INFO(this->get_logger(), "Charge requested: channel=%u, target_voltage=%u, id=%u", message.channel, message.target_voltage, message.event_info.id);

  if (session_not_started()) {
    RCLCPP_WARN(this->get_logger(), "Session not started. Can't charge coil.");
    return;
  }

  uint8_t error_value = event_interfaces::msg::ChargeError::NO_ERROR;
  if (!validate_charge_or_discharge(message.channel, message.target_voltage, error_value)) {
    event_interfaces::msg::ChargeFeedback feedback;
    feedback.id = message.event_info.id;
    feedback.error.value = error_value;
    charge_feedback_publisher_->publish(feedback);
    return;
  }

  wait_for_execution_condition(
    message.event_info.execution_condition,
    message.event_info.execution_time);

  event_interfaces::msg::ChargeFeedback feedback;
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    feedback = channels_[message.channel].charge(message.target_voltage, message.event_info.id);
  }
  charge_feedback_publisher_->publish(feedback);

  RCLCPP_INFO(this->get_logger(), "Charge completed: channel=%u, target_voltage=%u, id=%u", message.channel, message.target_voltage, message.event_info.id);
}

void MTMSSimulator::process_discharge(const event_interfaces::msg::Discharge & message)
{
  RCLCPP_INFO(this->get_logger(), "Discharge requested: channel=%u, target_voltage=%u, id=%u", message.channel, message.target_voltage, message.event_info.id);

  if (session_not_started()) {
    RCLCPP_WARN(this->get_logger(), "Session not started. Can't discharge coil.");
    return;
  }

  uint8_t error_value = event_interfaces::msg::DischargeError::NO_ERROR;
  if (!validate_charge_or_discharge(message.channel, message.target_voltage, error_value)) {
    event_interfaces::msg::DischargeFeedback feedback;
    feedback.id = message.event_info.id;
    feedback.error.value = error_value;
    discharge_feedback_publisher_->publish(feedback);
    return;
  }

  wait_for_execution_condition(
    message.event_info.execution_condition,
    message.event_info.execution_time);

  event_interfaces::msg::DischargeFeedback feedback;
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    feedback = channels_[message.channel].discharge(message.target_voltage, message.event_info.id);
  }
  discharge_feedback_publisher_->publish(feedback);

  RCLCPP_INFO(this->get_logger(), "Discharge completed: channel=%u, target_voltage=%u, id=%u", message.channel, message.target_voltage, message.event_info.id);
}

void MTMSSimulator::process_pulse(const event_interfaces::msg::Pulse & message)
{
  RCLCPP_INFO(this->get_logger(), "Pulse requested: channel=%u, id=%u", message.channel, message.event_info.id);

  wait_for_execution_condition(
    message.event_info.execution_condition,
    message.event_info.execution_time);

  auto error = validate_pulse(message);
  if (error.value != event_interfaces::msg::PulseError::NO_ERROR) {
    event_interfaces::msg::PulseFeedback feedback;
    feedback.id = message.event_info.id;
    feedback.error = error;
    pulse_feedback_publisher_->publish(feedback);
    return;
  }

  const auto total_duration = std::get<0>(calculate_waveform_durations(message.waveform));
  event_interfaces::msg::PulseFeedback feedback;
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    feedback = channels_[message.channel].pulse(
      message.event_info.id, static_cast<uint16_t>(total_duration));
  }
  pulse_feedback_publisher_->publish(feedback);

  RCLCPP_INFO(this->get_logger(), "Pulse completed: channel=%u, id=%u", message.channel, message.event_info.id);
}

void MTMSSimulator::process_trigger_out(const event_interfaces::msg::TriggerOut & message)
{
  (void)allow_trigger_out_;
  RCLCPP_INFO(this->get_logger(), "Trigger out requested: port=%u, id=%u, condition=%u, time=%.3f s", message.port, message.event_info.id,
    message.event_info.execution_condition.value, message.event_info.execution_time);

  wait_for_execution_condition(
    message.event_info.execution_condition,
    message.event_info.execution_time);

  if (neurone_trigger_client_) {
    neurone_trigger_client_->EnqueueTriggerForPort(message.port);
  }

  std::this_thread::sleep_for(std::chrono::duration<double>(message.duration_us / 1e6));

  event_interfaces::msg::TriggerOutFeedback feedback;
  feedback.id = message.event_info.id;
  feedback.error.value = event_interfaces::msg::TriggerOutError::NO_ERROR;
  trigger_out_feedback_publisher_->publish(feedback);

  RCLCPP_INFO(this->get_logger(), "Trigger out completed: port=%u, id=%u", message.port, message.event_info.id);
}

void MTMSSimulator::publish_system_state()
{
  mtms_device_interfaces::msg::SystemState msg;
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    msg = system_state_;
    for (size_t i = 0; i < num_of_channels_; ++i) {
      const auto & channel = channels_[i];
      mtms_device_interfaces::msg::ChannelState channel_state;
      channel_state.channel_index = static_cast<uint8_t>(clamp_uint(i, UINT8_MAX_V));
      channel_state.voltage = static_cast<uint16_t>(clamp_uint(channel.current_voltage(), UINT16_MAX_V));
      channel_state.temperature = static_cast<uint16_t>(clamp_uint(channel.temperature(), UINT16_MAX_V));
      channel_state.pulse_count = static_cast<uint32_t>(clamp_uint(channel.pulse_count, UINT32_MAX_V));
      channel_state.channel_error = channel.errors;
      msg.channel_states[i] = channel_state;
    }
  }
  system_state_publisher_->publish(msg);
}

void MTMSSimulator::publish_session()
{
  system_interfaces::msg::Session msg;
  msg.state = session_state_value_.load();
  msg.time = 0.0;
  if (msg.state == system_interfaces::msg::Session::STARTED) {
    msg.time = this->get_clock()->now().seconds() - session_start_time_.load();
  }
  session_publisher_->publish(msg);
}

void MTMSSimulator::queue_event_batch(
  const std::vector<event_interfaces::msg::Pulse> & pulses,
  const std::vector<event_interfaces::msg::Charge> & charges,
  const std::vector<event_interfaces::msg::Discharge> & discharges,
  const std::vector<event_interfaces::msg::TriggerOut> & trigger_outs)
{
  EventBatch batch;
  batch.pulses = pulses;
  batch.charges = charges;
  batch.discharges = discharges;
  batch.trigger_outs = trigger_outs;

  {
    std::lock_guard<std::mutex> lock(event_queue_mutex_);
    event_queue_.push_back(std::move(batch));
  }
  event_queue_cv_.notify_one();
}

void MTMSSimulator::event_worker_loop()
{
  while (true) {
    EventBatch batch;
    {
      std::unique_lock<std::mutex> lock(event_queue_mutex_);
      event_queue_cv_.wait(lock, [this]() {
        return stop_event_worker_ || !event_queue_.empty();
      });

      if (stop_event_worker_ && event_queue_.empty()) {
        return;
      }

      batch = std::move(event_queue_.front());
      event_queue_.pop_front();
    }

    for (const auto & pulse : batch.pulses) {
      process_pulse(pulse);
    }
    for (const auto & charge : batch.charges) {
      process_charge(charge);
    }
    for (const auto & discharge : batch.discharges) {
      process_discharge(discharge);
    }
    for (const auto & trigger_out : batch.trigger_outs) {
      process_trigger_out(trigger_out);
    }
  }
}

void MTMSSimulator::publish_healthcheck() const
{
  system_interfaces::msg::Healthcheck msg;
  msg.status = system_interfaces::msg::Healthcheck::READY;
  msg.status_message = "Simulator is ready";
  msg.actionable_message = "";
  healthcheck_publisher_->publish(msg);
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<MTMSSimulator>();

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}

