#include "timebase_calibrator.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/executors/single_threaded_executor.hpp"

#include "std_msgs/msg/empty.hpp"

using std::placeholders::_1;

static const std::string EEG_RAW_TOPIC           = "/mtms/eeg/raw";
static const std::string SESSION_TOPIC           = "/mtms/device/session";
static const std::string TIMEBASE_MAPPING_TOPIC  = "/mtms/timebase/mapping";
static const std::string HEARTBEAT_TOPIC         = "/mtms/timebase_calibrator/heartbeat";
constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

TimebaseCalibrator::TimebaseCalibrator()
: Node("timebase_calibrator")
{
  /* EEG is published with a large reliable queue; match that. */
  eeg_subscription = this->create_subscription<mtms_eeg_interfaces::msg::Sample>(
    EEG_RAW_TOPIC, 65535,
    std::bind(&TimebaseCalibrator::eeg_callback, this, _1));

  auto session_qos = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);

  session_subscription = this->create_subscription<mtms_system_interfaces::msg::Session>(
    SESSION_TOPIC, session_qos,
    std::bind(&TimebaseCalibrator::session_callback, this, _1));

  auto mapping_qos = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  mapping_publisher =
    this->create_publisher<mtms_system_interfaces::msg::TimebaseMapping>(
      TIMEBASE_MAPPING_TOPIC, mapping_qos);

  auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
  heartbeat_timer = this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
    heartbeat_publisher->publish(std_msgs::msg::Empty());
  });

  RCLCPP_INFO(this->get_logger(), "Timebase calibrator initialized.");
}

void TimebaseCalibrator::eeg_callback(const mtms_eeg_interfaces::msg::Sample::SharedPtr msg)
{
  latest_eeg_timestamp = msg->eeg_device_timestamp;

  if (!session_active) {
    return;
  }

  const double t = msg->eeg_device_timestamp;

  /* Detect missed trigger windows: advance state for each window whose end has passed. */
  while (!in_error_state && t > window_end) {
    consecutive_miss_count++;
    RCLCPP_WARN(this->get_logger(),
      "Missed expected sync trigger #%d (window [%.3f, %.3f] s, current %.3f s). Consecutive misses: %d",
      expected_trigger_number, window_start, window_end, t, consecutive_miss_count);

    if (consecutive_miss_count >= 2) {
      in_error_state = true;
      RCLCPP_ERROR(this->get_logger(),
        "Two consecutive missed sync triggers. Entering error state until session ends.");
      break;
    }

    /* Advance to next expected trigger window (1 s later). */
    expected_trigger_number++;
    window_start += 1.0;
    window_end   += 1.0;
  }

  /* Only process sync triggers. */
  if (!msg->trigger_b) {
    return;
  }

  if (in_error_state) {
    return;
  }

  /* Ignore triggers that arrive before the expected window. */
  if (t < window_start) {
    RCLCPP_WARN(this->get_logger(),
      "Sync trigger at %.3f s is before expected window [%.3f, %.3f] s; ignoring.",
      t, window_start, window_end);
    return;
  }

  /* Trigger received within the expected window. */
  const double mtms_session_time = static_cast<double>(expected_trigger_number);

  RCLCPP_INFO(this->get_logger(),
    "Sync trigger #%d: eeg_device_timestamp=%.6f s -> mtms_session_time=%.1f s",
    expected_trigger_number, t, mtms_session_time);

  mtms_system_interfaces::msg::TimebaseMapping mapping;
  mapping.valid = true;
  mapping.eeg_device_timestamp = t;
  mapping.mtms_session_time = mtms_session_time;
  mapping_publisher->publish(mapping);

  /* Reset miss counter and set window for next expected trigger. */
  consecutive_miss_count = 0;
  expected_trigger_number++;
  window_start = t + 0.5;
  window_end   = t + 1.5;
}

void TimebaseCalibrator::session_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg)
{
  const bool started = (msg->state == mtms_system_interfaces::msg::Session::STARTED);

  if (started && !session_active) {
    session_active = true;

    /* Expect the first sync trigger 0.5–1.5 s after the latest EEG timestamp. */
    window_start           = latest_eeg_timestamp + 0.5;
    window_end             = latest_eeg_timestamp + 1.5;
    expected_trigger_number = 1;
    consecutive_miss_count  = 0;
    in_error_state          = false;

    RCLCPP_INFO(this->get_logger(),
      "Session started. Expecting first sync trigger in window [%.3f, %.3f] s.",
      window_start, window_end);
  }
  if (!started && session_active) {
    /* Session is stopping or has stopped - invalidate the mapping. */
    session_active = false;

    mtms_system_interfaces::msg::TimebaseMapping mapping;
    mapping.valid = false;
    mapping_publisher->publish(mapping);

    RCLCPP_INFO(this->get_logger(), "Session no longer active; calibration invalidated.");
  }
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<TimebaseCalibrator>();

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
