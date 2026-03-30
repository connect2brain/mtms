#include "remote_controller/remote_controller.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <thread>
#include <string>
#include <utility>

#include "std_srvs/srv/trigger.hpp"

#include "rclcpp/executors/single_threaded_executor.hpp"

#include "realtime_utils/utils.h"

#include "std_msgs/msg/empty.hpp"
#include "std_msgs/msg/bool.hpp"

using namespace std::chrono_literals;

using std::placeholders::_1;

const std::string SESSION_TOPIC = "/mtms/device/session";
const std::string EEG_TO_MTMS_TOPIC = "/mtms/timebase/eeg_to_mtms";
const std::string TARGETED_PULSES_TOPIC = "/mtms/targeted_pulses";
const std::string PERFORM_TRIAL_SERVICE = "/mtms/trial/perform";
const std::string CACHE_TARGET_LIST_SERVICE = "/mtms/trial/cache";
const std::string PREPARE_TRIAL_SERVICE = "/mtms/trial/prepare";
const std::string START_SESSION_SERVICE = "/mtms/device/session/start";
const std::string STOP_SESSION_SERVICE = "/mtms/device/session/stop";
const std::string HEARTBEAT_TOPIC = "/mtms/remote_controller/heartbeat";
const std::string REMOTE_CONTROLLER_STATE_TOPIC = "/mtms/remote_controller/state";
const std::string TRIAL_READINESS_TOPIC = "/mtms/trial/trial_readiness";

constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

RemoteController::RemoteController(const rclcpp::NodeOptions & options)
: Node("remote_controller", options)
{
  const auto latched_qos = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  state_publisher =
    this->create_publisher<mtms_trial_interfaces::msg::RemoteControllerState>(
      REMOTE_CONTROLLER_STATE_TOPIC, latched_qos);

  /* Start and stop services. */
  start_service = this->create_service<mtms_trial_interfaces::srv::StartRemoteController>(
    "/mtms/remote_controller/start",
    std::bind(&RemoteController::start_service_handler, this, _1, std::placeholders::_2));

  stop_service = this->create_service<std_srvs::srv::Trigger>(
    "/mtms/remote_controller/stop",
    std::bind(&RemoteController::stop_service_handler, this, _1, std::placeholders::_2));

  /* Subscription for eeg_to_mtms mapping. */
  eeg_to_mtms_subscriber = this->create_subscription<mtms_system_interfaces::msg::TimebaseMapping>(
    EEG_TO_MTMS_TOPIC,
    latched_qos,
    std::bind(&RemoteController::eeg_to_mtms_callback, this, _1));

  /* Subscription for targeted pulses. */
  const auto pulses_qos = rclcpp::QoS(rclcpp::KeepLast(10))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);

  targeted_pulses_subscriber = this->create_subscription<shared_stimulation_interfaces::msg::TargetedPulses>(
    TARGETED_PULSES_TOPIC,
    pulses_qos,
    std::bind(&RemoteController::targeted_pulses_callback, this, _1));

  /* Subscription for trial readiness. */
  trial_readiness_subscriber = this->create_subscription<std_msgs::msg::Bool>(
    TRIAL_READINESS_TOPIC,
    latched_qos,
    std::bind(&RemoteController::trial_readiness_callback, this, _1));

  /* Subscription for session state. */
  const auto session_qos = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);

  session_subscriber = this->create_subscription<mtms_system_interfaces::msg::Session>(
    SESSION_TOPIC,
    session_qos,
    std::bind(&RemoteController::session_state_callback, this, _1));

  /* Service client for performing trials. */
  perform_trial_client =
    this->create_client<mtms_trial_interfaces::srv::PerformTrial>(PERFORM_TRIAL_SERVICE);

  while (!perform_trial_client->wait_for_service(std::chrono::seconds(2))) {
    RCLCPP_INFO(this->get_logger(), "Waiting for service '%s' to become available...", PERFORM_TRIAL_SERVICE.c_str());
  }

  /* Service client for caching trials. */
  cache_target_list_client =
    this->create_client<mtms_trial_interfaces::srv::CacheTargetList>(CACHE_TARGET_LIST_SERVICE);

  while (!cache_target_list_client->wait_for_service(std::chrono::seconds(2))) {
    RCLCPP_INFO(this->get_logger(), "Waiting for service '%s' to become available...", CACHE_TARGET_LIST_SERVICE.c_str());
  }

  /* Service client for starting the session. */
  start_session_client =
    this->create_client<mtms_system_interfaces::srv::StartSession>(START_SESSION_SERVICE);

  while (!start_session_client->wait_for_service(std::chrono::seconds(2))) {
    RCLCPP_INFO(this->get_logger(), "Waiting for service '%s' to become available...", START_SESSION_SERVICE.c_str());
  }

  /* Service client for stopping the session. */
  stop_session_client =
    this->create_client<mtms_system_interfaces::srv::StopSession>(STOP_SESSION_SERVICE);

  while (!stop_session_client->wait_for_service(std::chrono::seconds(2))) {
    RCLCPP_INFO(this->get_logger(), "Waiting for service '%s' to become available...", STOP_SESSION_SERVICE.c_str());
  }

  /* Service client for preparing trials. */
  prepare_trial_client =
    this->create_client<std_srvs::srv::Trigger>(PREPARE_TRIAL_SERVICE);
  while (!prepare_trial_client->wait_for_service(std::chrono::seconds(2))) {
    RCLCPP_INFO(this->get_logger(), "Waiting for service '%s' to become available...", PREPARE_TRIAL_SERVICE.c_str());
  }

  auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
  heartbeat_timer = this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
    heartbeat_publisher->publish(std_msgs::msg::Empty());
  });

  set_state(mtms_trial_interfaces::msg::RemoteControllerState::NOT_STARTED);

  RCLCPP_INFO(this->get_logger(), "Remote controller initialized");
}

void RemoteController::set_state(uint8_t new_state)
{
  RCLCPP_INFO(this->get_logger(), "Setting remote controller state to %d", new_state);

  {
    std::lock_guard<std::mutex> lock(state_mutex);
    this->state.state = new_state;
  }
  state_publisher->publish(this->state);
}

uint8_t RemoteController::get_state()
{
  std::lock_guard<std::mutex> lock(state_mutex);
  return this->state.state;
}

void RemoteController::start_service_handler(
  const std::shared_ptr<mtms_trial_interfaces::srv::StartRemoteController::Request> request,
  std::shared_ptr<mtms_trial_interfaces::srv::StartRemoteController::Response> response)
{
  if (get_state() != mtms_trial_interfaces::msg::RemoteControllerState::NOT_STARTED) {
    RCLCPP_ERROR(this->get_logger(), "Remote controller is not in NOT_STARTED state; cannot start.");
    response->success = false;
    return;
  }

  std::vector<mtms_trial_interfaces::msg::TargetList> target_lists(
    request->target_lists.begin(), request->target_lists.end());

  stored_target_lists.clear();
  for (const auto & tl : target_lists) {
    stored_target_lists.push_back(
      std::vector<mtms_targeting_interfaces::msg::ElectricTarget>(
        tl.targets.begin(), tl.targets.end()));
  }

  std::thread([this, target_lists = std::move(target_lists)]() mutable {
    set_state(mtms_trial_interfaces::msg::RemoteControllerState::CACHING);
    cache_target_lists_async(std::move(target_lists));

    set_state(mtms_trial_interfaces::msg::RemoteControllerState::STARTING);

    /* Start the device. */
    if (!start_session()) {
      RCLCPP_ERROR(this->get_logger(), "StartSession did not succeed; state will remain NOT_STARTED.");
      set_state(mtms_trial_interfaces::msg::RemoteControllerState::NOT_STARTED);
      return;
    }

    /* Start preparing the trial. */
    prepare_trial();

    /* Wait for trial readiness. */
    while (!trial_readiness) {
      std::this_thread::sleep_for(50ms);
    }

    /* Set the state to STARTED. */
    set_state(mtms_trial_interfaces::msg::RemoteControllerState::STARTED);
  }).detach();

  response->success = true;
  RCLCPP_INFO(this->get_logger(), "Remote controller started");
}

void RemoteController::stop_service_handler(
  [[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  (void)request;
  if (get_state() != mtms_trial_interfaces::msg::RemoteControllerState::STARTED) {
    RCLCPP_ERROR(this->get_logger(), "Remote controller is not in STARTED state; cannot stop.");
    response->success = false;
    return;
  }

  stored_target_lists.clear();

  std::thread([this]() {
    set_state(mtms_trial_interfaces::msg::RemoteControllerState::STOPPING);

    /* Wait until trial is not being prepared before stopping the session. */
    while (this->prepare_trial_ongoing) {
      std::this_thread::sleep_for(100ms);
    }

    /* Stop the session. */
    if (!stop_session()) {
      RCLCPP_ERROR(this->get_logger(), "StopSession did not succeed.");
      return;
    }

    /* Set the state to NOT_STARTED. */
    set_state(mtms_trial_interfaces::msg::RemoteControllerState::NOT_STARTED);

    RCLCPP_INFO(this->get_logger(), "Remote controller stopped");
  }).detach();

  response->success = true;
}

void RemoteController::cache_target_lists_async(std::vector<mtms_trial_interfaces::msg::TargetList> target_lists)
{
  RCLCPP_INFO(this->get_logger(), "Caching %zu target list(s)...", target_lists.size());
  for (size_t i = 0; i < target_lists.size(); ++i) {
    auto request = std::make_shared<mtms_trial_interfaces::srv::CacheTargetList::Request>();
    request->targets = target_lists[i].targets;

    auto future = cache_target_list_client->async_send_request(request);
    try {
      auto response = future.get();
      if (response->success) {
        RCLCPP_INFO(this->get_logger(), "Target list %zu cached successfully.", i + 1);
      } else {
        RCLCPP_WARN(this->get_logger(), "CacheTargetList returned failure for target list %zu.", i + 1);
      }
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "CacheTargetList failed for target list %zu: %s", i + 1, e.what());
    }
  }
  RCLCPP_INFO(this->get_logger(), "Done caching target lists.");
}

bool RemoteController::is_trial_target_list_compatible(const mtms_trial_interfaces::msg::Trial & trial)
{
  for (const auto & stored : stored_target_lists) {
    if (stored == trial.targets) {
      return true;
    }
  }
  return false;
}

void RemoteController::session_state_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg)
{
  is_session_started = (msg->state == mtms_system_interfaces::msg::Session::STARTED || msg->state == mtms_system_interfaces::msg::Session::STOPPING);
}

bool RemoteController::start_session()
{
  auto request = std::make_shared<mtms_system_interfaces::srv::StartSession::Request>();
  auto future = start_session_client->async_send_request(request);
  auto response = future.get();
  if (!response || !response->success) {
    return false;
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
  while (!is_session_started) {
    if (std::chrono::steady_clock::now() > deadline) {
      RCLCPP_ERROR(this->get_logger(), "Timed out waiting for session to reach STARTED state.");
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return true;
}

bool RemoteController::stop_session()
{
  auto request = std::make_shared<mtms_system_interfaces::srv::StopSession::Request>();
  auto future = stop_session_client->async_send_request(request);
  auto response = future.get();
  if (!response || !response->success) {
    return false;
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
  while (is_session_started) {
    if (std::chrono::steady_clock::now() > deadline) {
      RCLCPP_ERROR(this->get_logger(), "Timed out waiting for session to reach STOPPED state.");
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return true;
}

void RemoteController::eeg_to_mtms_callback(const mtms_system_interfaces::msg::TimebaseMapping::SharedPtr msg)
{
  latest_eeg_to_mtms = *msg;
}

int8_t RemoteController::clamp_displacement_mm_to_int8(double mm)
{
  const long long rounded = static_cast<long long>(std::llround(mm));
  const long long clamped = std::clamp<long long>(
    rounded,
    -static_cast<long long>(MAX_ABSOLUTE_DISPLACEMENT_MM),
    static_cast<long long>(MAX_ABSOLUTE_DISPLACEMENT_MM));
  return static_cast<int8_t>(clamped);
}

int16_t RemoteController::wrap_rotation_deg_to_int16(double degrees)
{
  // Message rotation is defined as [0, 360), but be robust to small numerical noise.
  const long long rounded = static_cast<long long>(std::llround(degrees));
  long long wrapped = rounded % 360LL;
  if (wrapped < 0) {
    wrapped += 360LL;
  }

  const long long clamped = std::clamp<long long>(
    wrapped,
    0LL,
    static_cast<long long>(MAX_ROTATION_ANGLE_DEG));
  return static_cast<int16_t>(clamped);
}

uint8_t RemoteController::clamp_intensity_to_uint8(double intensity_v_m)
{
  const long long rounded = static_cast<long long>(std::llround(intensity_v_m));
  const long long clamped = std::clamp<long long>(
    rounded,
    0LL,
    static_cast<long long>(INTENSITY_LIMIT_V_M));
  return static_cast<uint8_t>(clamped);
}

bool RemoteController::build_trial_from_message(
  const shared_stimulation_interfaces::msg::TargetedPulses & msg,
  const mtms_system_interfaces::msg::TimebaseMapping & mapping,
  mtms_trial_interfaces::msg::Trial & trial_out) const
{
  if (msg.pulses.empty()) {
    return false;
  }
  if (!std::isfinite(mapping.scale) || !std::isfinite(mapping.offset) || mapping.scale == 0.0) {
    return false;
  }

  trial_out.targets.clear();
  trial_out.pulse_times_since_trial_start.clear();
  trial_out.trigger_enabled.clear();
  trial_out.trigger_delay.clear();

  trial_out.targets.reserve(msg.pulses.size());
  trial_out.pulse_times_since_trial_start.reserve(msg.pulses.size());

  // Mapping: mtms_time = scale * eeg_time + offset.
  // Each pulse time in `TargetedPulses` is relative to `reference_eeg_device_timestamp`.
  const double ref_eeg_time_s = msg.reference_eeg_device_timestamp;
  trial_out.start_time = mapping.scale * ref_eeg_time_s + mapping.offset;

  for (const auto & pulse : msg.pulses) {
    mtms_targeting_interfaces::msg::ElectricTarget target;
    target.displacement_x = clamp_displacement_mm_to_int8(pulse.displacement_x);
    target.displacement_y = clamp_displacement_mm_to_int8(pulse.displacement_y);
    target.rotation_angle = wrap_rotation_deg_to_int16(pulse.rotation_angle);
    target.intensity = clamp_intensity_to_uint8(pulse.intensity);
    target.algorithm = mtms_targeting_interfaces::msg::ElectricTarget::LEAST_SQUARES;

    trial_out.targets.push_back(target);
    trial_out.pulse_times_since_trial_start.push_back(mapping.scale * pulse.time_offset);
  }

  return true;
}

void RemoteController::targeted_pulses_callback(const shared_stimulation_interfaces::msg::TargetedPulses::SharedPtr msg)
{
  if (!get_state() == mtms_trial_interfaces::msg::RemoteControllerState::STARTED) {
    RCLCPP_WARN_THROTTLE(
      this->get_logger(),
      *this->get_clock(),
      5000,
      "Remote controller not started; ignoring TargetedPulses.");
    return;
  }

  mtms_system_interfaces::msg::TimebaseMapping mapping;
  if (!latest_eeg_to_mtms.has_value()) {
    RCLCPP_WARN_THROTTLE(
      this->get_logger(),
      *this->get_clock(),
      5000,
      "No eeg_to_mtms mapping received yet; cannot build trial.");
    return;
  }
  mapping = *latest_eeg_to_mtms;

  mtms_trial_interfaces::msg::Trial trial;
  if (!build_trial_from_message(*msg, mapping, trial)) {
    RCLCPP_WARN(this->get_logger(), "Failed to build a valid Trial from incoming TargetedPulses.");
    return;
  }

  if (!is_trial_target_list_compatible(trial)) {
    RCLCPP_WARN(
      this->get_logger(),
      "Trial target list does not match any cached target list; skipping TargetedPulses.");
    return;
  }

  if (trial_ongoing) {
    RCLCPP_WARN(
      this->get_logger(),
      "Previous PerformTrial request is still running; skipping TargetedPulses.");
    return;
  }
  trial_ongoing = true;

  auto request = std::make_shared<mtms_trial_interfaces::srv::PerformTrial::Request>();
  request->trial = trial;

  auto future = perform_trial_client->async_send_request(request);

  // Detach so we don't block the subscription callback while the trial is executing.
  std::thread([this, future = std::move(future)]() mutable {
    try {
      auto response = future.get();
      if (response) {
        RCLCPP_INFO(
          this->get_logger(),
          "PerformTrial completed (success=%s).",
          response->success ? "true" : "false");
      } else {
        RCLCPP_WARN(this->get_logger(), "PerformTrial returned null response.");
      }
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "PerformTrial failed: %s", e.what());
    }
    trial_ongoing = false;
  }).detach();
}

void RemoteController::trial_readiness_callback(const std_msgs::msg::Bool::SharedPtr msg)
{
  this->trial_readiness = msg->data;
  if (get_state() != mtms_trial_interfaces::msg::RemoteControllerState::STARTED) {
    return;
  }
  if (!trial_readiness) {
    RCLCPP_INFO(this->get_logger(), "Trial readiness is false; preparing trial.");
    prepare_trial();
  }
}

void RemoteController::prepare_trial()
{
  if (this->prepare_trial_ongoing) {
    return;
  }
  this->prepare_trial_ongoing = true;

  auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
  auto future = prepare_trial_client->async_send_request(request);

  std::thread([this, future = std::move(future)]() mutable {
    auto response = future.get();
    if (response && response->success) {
      RCLCPP_INFO(this->get_logger(), "Prepare trial completed successfully.");
    } else {
      RCLCPP_WARN(this->get_logger(), "Prepare trial completed with failure (success=false).");
    }

    this->prepare_trial_ongoing = false;
  }).detach();
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto logger = rclcpp::get_logger("remote_controller");

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

  rclcpp::executors::SingleThreadedExecutor executor;
  auto node = std::make_shared<RemoteController>();
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
