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

/* Maximum age (in seconds) of a sync trigger to be considered fresh when a
   session starts. Measured as (latest_eeg_timestamp - trigger_eeg_timestamp). */
static constexpr double STALE_THRESHOLD_S = 1.0;

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

  /* Only process sync triggers. */
  if (!msg->trigger_b) {
    return;
  }

  /* Always track the most recent sync trigger timestamp, even outside a session,
     so we can check staleness when a session starts. */
  last_sync_trigger_eeg_timestamp = msg->eeg_device_timestamp;

  /* Only process sync triggers if a session is active. */
  if (!session_active) {
    return;
  }

  sync_trigger_count++;

  /* Each sync trigger corresponds to one second of mTMS session time, hence the conversion is straightforward. */
  const double mtms_session_time = static_cast<double>(sync_trigger_count);

  RCLCPP_INFO(
    this->get_logger(),
    "Sync trigger #%d: eeg_device_timestamp=%.6f s -> mtms_session_time=%.1f s",
    sync_trigger_count, msg->eeg_device_timestamp, mtms_session_time);

  mtms_system_interfaces::msg::TimebaseMapping mapping;
  mapping.valid = true;
  mapping.eeg_device_timestamp = msg->eeg_device_timestamp;
  mapping.mtms_session_time = mtms_session_time;
  mapping_publisher->publish(mapping);
}

void TimebaseCalibrator::session_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg)
{
  const bool started = (msg->state == mtms_system_interfaces::msg::Session::STARTED);

  if (started && !session_active) {
    session_active = true;
    sync_trigger_count = -1;

    /* Check if the most recent sync trigger is fresh enough to serve as t=0. */
    if (last_sync_trigger_eeg_timestamp.has_value() &&
        (latest_eeg_timestamp - *last_sync_trigger_eeg_timestamp) < STALE_THRESHOLD_S)
    {
      RCLCPP_INFO(this->get_logger(),
        "Session started. Recent sync trigger (age %.3f s); using as trigger 0.",
        latest_eeg_timestamp - *last_sync_trigger_eeg_timestamp);

      sync_trigger_count++;

      mtms_system_interfaces::msg::TimebaseMapping mapping;
      mapping.valid = true;
      mapping.eeg_device_timestamp = *last_sync_trigger_eeg_timestamp;
      mapping.mtms_session_time = 0.0;
      mapping_publisher->publish(mapping);
    } else {
      RCLCPP_INFO(this->get_logger(),
      "Session started. No recent sync trigger; waiting for first trigger_b.");
    }
  }
  if (!started && session_active) {
    /* Session is stopping or has stopped - invalidate the mapping. */
    session_active = false;
    sync_trigger_count = -1;

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
