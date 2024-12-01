#include <chrono>
#include <thread>

#include <LabJackM.h>
#include "LJM_Utilities.h"

#include "trigger_timer.h"

#include "memory_utils.h"
#include "scheduling_utils.h"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::placeholders;

const std::string TIMED_TRIGGER_SERVICE = "/pipeline/timed_trigger";
const std::string EEG_RAW_TOPIC = "/eeg/raw";

const double_t latency_measurement_interval = 0.1;
const double_t maximum_triggering_error = 0.003;

/* Note: Needs to match the values in session_bridge.cpp. */
const milliseconds SESSION_PUBLISHING_INTERVAL = 20ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 5ms;

const char* tms_trigger_fio = "FIO5";
const char* latency_measurement_trigger_fio = "FIO4";

TriggerTimer::TriggerTimer() : Node("trigger_timer"), logger(rclcpp::get_logger("trigger_timer")) {
  /* Subscriber for mTMS device healthcheck. */
  this->mtms_device_healthcheck_subscriber = create_subscription<system_interfaces::msg::Healthcheck>(
    "/mtms_device/healthcheck",
    10,
    std::bind(&TriggerTimer::handle_mtms_device_healthcheck, this, _1));

  /* Subscriber for EEG raw data. */
  this->eeg_raw_subscriber = create_subscription<eeg_interfaces::msg::Sample>(
    EEG_RAW_TOPIC,
    10,
    std::bind(&TriggerTimer::handle_eeg_raw, this, _1));

  /* Subscriber for session. */
  const auto DEADLINE_NS = std::chrono::nanoseconds(SESSION_PUBLISHING_INTERVAL + SESSION_PUBLISHING_INTERVAL_TOLERANCE);

  auto qos_session = rclcpp::QoS(rclcpp::KeepLast(1))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
      .deadline(DEADLINE_NS)
      .lifespan(DEADLINE_NS);

  rclcpp::SubscriptionOptions subscription_options;
  subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Session not received within deadline.");
  };

  this->session_subscriber = this->create_subscription<system_interfaces::msg::Session>(
    "/system/session",
    qos_session,
    std::bind(&TriggerTimer::handle_session, this, _1),
    subscription_options);

  this->latency_measurement_trigger_subscriber = create_subscription<system_interfaces::msg::TimedTrigger>(
    "/pipeline/latency_measurement_trigger",
    10,
    std::bind(&TriggerTimer::handle_latency_measurement_trigger, this, _1));

  /* Service for trigger request. */
  this->trigger_request_service = create_service<system_interfaces::srv::RequestTimedTrigger>(
    TIMED_TRIGGER_SERVICE,
    std::bind(&TriggerTimer::handle_request_timed_trigger, this, _1, _2));

  /* Publisher for timing latency. */
  this->timing_latency_publisher = this->create_publisher<pipeline_interfaces::msg::TimingLatency>(
    "/pipeline/timing/latency",
    10);

  /* Publisher for decision info. */
  this->decision_info_publisher = this->create_publisher<pipeline_interfaces::msg::DecisionInfo>(
    "/pipeline/decision_info",
    10);

  /* Attempt initial connection to LabJack. */
  attempt_labjack_connection();

  /* Set up a timer to attempt reconnection every second. */
  timer = this->create_wall_timer(
    std::chrono::seconds(1),
    std::bind(&TriggerTimer::attempt_labjack_connection, this));
}

TriggerTimer::~TriggerTimer() {
  if (labjack_handle != -1) {
    CloseOrDie(labjack_handle);
  }
}

void TriggerTimer::handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg) {
  bool state_changed = this->session_state.value != msg->state.value;
  this->session_state = msg->state;

  if (state_changed) {
    if (this->session_state.value == system_interfaces::msg::SessionState::STOPPED) {
      RCLCPP_INFO(this->get_logger(), "Session stopped, resetting.");
      this->current_latency = 0.0;
      this->last_latency_measurement_time = 0.0;
    }
  }
}

void TriggerTimer::handle_latency_measurement_trigger(const std::shared_ptr<system_interfaces::msg::TimedTrigger> msg) {
  double_t trigger_time = msg->time;

  current_latency = trigger_time - last_latency_measurement_time;

  /* Publish latency ROS message. */
  auto msg_ = pipeline_interfaces::msg::TimingLatency();
  msg_.latency = current_latency;

  this->timing_latency_publisher->publish(msg_);
}

void TriggerTimer::attempt_labjack_connection() {
  if (labjack_handle == -1) {
    /* Attempt to open a connection to the LabJack device. */
    int err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &labjack_handle);
    if (err != LJME_NOERROR) {
      /* Ensure that handle is marked as invalid. */
      labjack_handle = -1;
      RCLCPP_WARN(logger, "Failed to connect to LabJack. Error code: %d", err);
    } else {
      PrintDeviceInfoFromHandle(labjack_handle);
      RCLCPP_INFO(logger, "Successfully connected to LabJack.");
    }
  }
}

bool TriggerTimer::safe_error_check(int err, const char* action) {
  if (err != LJME_NOERROR) {
    RCLCPP_ERROR(logger, "%s failed with error code: %d", action, err);
    if (err == LJME_RECONNECT_FAILED) {
      /* Mark as disconnected. */
      labjack_handle = -1;
      RCLCPP_WARN(logger, "LabJack connection lost. Will attempt to reconnect.");
    }
    return false;
  }
  return true;
}

void TriggerTimer::handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) {
  this->mtms_device_available = msg->status.value == system_interfaces::msg::HealthcheckStatus::READY;
}

void TriggerTimer::handle_eeg_raw(const std::shared_ptr<eeg_interfaces::msg::Sample> msg) {
  double_t current_time = msg->time;

  double_t latency_corrected_time = current_time + current_latency;

  std::lock_guard<std::mutex> lock(queue_mutex);

  /* Trigger all events that are due. */
  while (!trigger_queue.empty() && trigger_queue.top() <= latency_corrected_time) {
    double_t scheduled_time = trigger_queue.top();
    double_t error = latency_corrected_time - scheduled_time;

    if (std::abs(error) <= maximum_triggering_error) {
      RCLCPP_INFO(logger, "Triggering at time: %.4f (current time: %.4f, error: %.4f)", scheduled_time, latency_corrected_time, error);
      trigger_labjack(tms_trigger_fio);
    } else {
      RCLCPP_WARN(logger, "Skipping trigger at time: %.4f (current time: %.4f, error: %.4f exceeds threshold: %.4f)",
                  scheduled_time, latency_corrected_time, error, maximum_triggering_error);
    }

    trigger_queue.pop();
  }

  /* Trigger latency measurement event at specific intervals. */
  if (current_time - last_latency_measurement_time >= latency_measurement_interval) {
    trigger_labjack(latency_measurement_trigger_fio);
    last_latency_measurement_time = current_time;
  }
}

void TriggerTimer::handle_request_timed_trigger(
    const std::shared_ptr<system_interfaces::srv::RequestTimedTrigger::Request> request,
    std::shared_ptr<system_interfaces::srv::RequestTimedTrigger::Response> response) {

  double_t trigger_time = request->timed_trigger.time;

  /* Add the trigger time to the queue. */
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    trigger_queue.push(trigger_time);
  }

  /* Create and publish decision info. */
  auto msg = pipeline_interfaces::msg::DecisionInfo();
  msg.stimulate = true;

  msg.decision_time = request->decision_time;
  msg.decider_latency = request->decider_latency;
  msg.preprocessor_latency = request->preprocessor_latency;

  /* In case of a positive stimulation decision, the total latency can only be added Trigger Timer -
     it cannot be calculated by Decider due to the additional component (Trigger Timer) on the pathway. */
  rclcpp::Time now = this->get_clock()->now();
  rclcpp::Time sample_time_rcl(request->system_time_for_sample);
  double_t total_latency = now.seconds() - sample_time_rcl.seconds();

  msg.total_latency = total_latency;

  this->decision_info_publisher->publish(msg);

  RCLCPP_INFO(logger, "Scheduled trigger at time: %.4f, total decision-making latency: %.4f", trigger_time, total_latency);
  response->success = true;
}

void TriggerTimer::trigger_labjack(const char* name) {
  if (labjack_handle == -1) {
    RCLCPP_WARN(logger, "LabJack is not connected. Skipping trigger.");
    return;
  }

  /* Set output port state to high. */
  int err = LJM_eWriteName(labjack_handle, name, 1);
  if (!safe_error_check(err, "Setting digital output on LabJack")) {
    return;
  }

  /* Wait for one millisecond. */
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  /* Set output port state to low. */
  err = LJM_eWriteName(labjack_handle, name, 0);
  if (!safe_error_check(err, "Setting digital output on LabJack")) {
    return;
  }

  RCLCPP_INFO(logger, "Triggered LabJack on output port %s", name);
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("trigger_timer"), "Setting real-time priority (%d)", DEFAULT_REALTIME_SCHEDULING_PRIORITY);
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<TriggerTimer>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("trigger_timer"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
