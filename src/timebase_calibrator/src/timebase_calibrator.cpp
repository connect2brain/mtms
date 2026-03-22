#include "timebase_calibrator.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/executors/single_threaded_executor.hpp"

using std::placeholders::_1;

static const std::string EEG_RAW_TOPIC      = "/mtms/eeg/raw";
static const std::string SESSION_TOPIC      = "/mtms/device/session";
static const std::string EEG_TO_MTMS_TOPIC  = "/mtms/timebase/eeg_to_mtms";
static const std::string MTMS_TO_EEG_TOPIC  = "/mtms/timebase/mtms_to_eeg";

TimebaseCalibrator::TimebaseCalibrator()
: Node("timebase_calibrator")
{
  pairs.reset(PAIR_BUFFER_SIZE);

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

  eeg_to_mtms_publisher =
    this->create_publisher<mtms_system_interfaces::msg::TimebaseMapping>(
      EEG_TO_MTMS_TOPIC, mapping_qos);

  mtms_to_eeg_publisher =
    this->create_publisher<mtms_system_interfaces::msg::TimebaseMapping>(
      MTMS_TO_EEG_TOPIC, mapping_qos);

  RCLCPP_INFO(this->get_logger(), "Timebase calibrator initialized. "
    "Listening to '%s' and '%s'. Buffer size: %zu pairs.",
    EEG_RAW_TOPIC.c_str(), SESSION_TOPIC.c_str(), PAIR_BUFFER_SIZE);
}

void TimebaseCalibrator::eeg_callback(const mtms_eeg_interfaces::msg::Sample::SharedPtr msg)
{
  std::lock_guard<std::mutex> lock(mutex);
  latest_eeg_sample = *msg;
}

void TimebaseCalibrator::session_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg)
{
  std::lock_guard<std::mutex> lock(mutex);

  if (!latest_eeg_sample.has_value()) {
    RCLCPP_WARN_THROTTLE(
      this->get_logger(), *this->get_clock(), 5000,
      "Session received but no EEG sample has arrived yet; skipping pair.");
    return;
  }

  SamplePair pair;
  pair.eeg_sample = *latest_eeg_sample;
  pair.session = *msg;
  pairs.append(pair);

  RCLCPP_DEBUG(
    this->get_logger(),
    "Pair recorded: EEG sample_index=%lu, eeg_time=%.6f s, session_time=%.6f s",
    pair.eeg_sample.sample_index,
    pair.eeg_sample.time,
    pair.session.time);

  double scale, offset;
  if (!compute_lms(scale, offset)) {
    return;
  }

  mtms_system_interfaces::msg::TimebaseMapping eeg_to_mtms_msg;
  eeg_to_mtms_msg.scale  = scale;
  eeg_to_mtms_msg.offset = offset;
  eeg_to_mtms_publisher->publish(eeg_to_mtms_msg);

  /* Inverse mapping: eeg_time = (session_time - offset) / scale */
  mtms_system_interfaces::msg::TimebaseMapping mtms_to_eeg_msg;
  mtms_to_eeg_msg.scale  = 1.0 / scale;
  mtms_to_eeg_msg.offset = -offset / scale;
  mtms_to_eeg_publisher->publish(mtms_to_eeg_msg);
}

bool TimebaseCalibrator::compute_lms(double & scale, double & offset) const
{
  std::vector<SamplePair> snapshot;
  snapshot.reserve(PAIR_BUFFER_SIZE);
  pairs.process_elements([&snapshot](const SamplePair & p) {
    snapshot.push_back(p);
  });

  const double n = static_cast<double>(snapshot.size());
  if (n < 2) {
    return false;
  }

  double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
  for (const auto & p : snapshot) {
    const double x = p.eeg_sample.time;
    const double y = p.session.time;
    sum_x  += x;
    sum_y  += y;
    sum_xy += x * y;
    sum_xx += x * x;
  }

  const double denom = n * sum_xx - sum_x * sum_x;
  if (denom == 0.0) {
    RCLCPP_WARN(this->get_logger(), "LMS fit degenerate (all EEG timestamps identical); skipping.");
    return false;
  }

  scale  = (n * sum_xy - sum_x * sum_y) / denom;
  offset = (sum_y - scale * sum_x) / n;

  RCLCPP_DEBUG(
    this->get_logger(),
    "LMS fit over %zu pairs: scale=%.9f, offset=%.6f s",
    snapshot.size(), scale, offset);

  return true;
}


std::vector<SamplePair> TimebaseCalibrator::get_pairs() const
{
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<SamplePair> result;
  result.reserve(PAIR_BUFFER_SIZE);
  pairs.process_elements([&result](const SamplePair & p) {
    result.push_back(p);
  });
  return result;
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
