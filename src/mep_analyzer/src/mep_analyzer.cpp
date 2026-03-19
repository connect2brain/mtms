#include "mep_analyzer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <thread>

using namespace std::chrono_literals;

const std::string MepAnalyzerNode::EEG_RAW_TOPIC = "/mtms/eeg/raw";
const std::string MepAnalyzerNode::EEG_DEVICE_INFO_TOPIC = "/mtms/eeg_device/info";
const std::string MepAnalyzerNode::SERVICE_ANALYZE_MEP = "/mtms/mep/analyze";

MepAnalyzerNode::MepAnalyzerNode(const rclcpp::NodeOptions & options)
: Node("mep_analyzer", options)
{
  callback_group = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

  const auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  device_info_subscriber = this->create_subscription<eeg_interfaces::msg::EegDeviceInfo>(
    EEG_DEVICE_INFO_TOPIC.c_str(),
    qos_persist_latest,
    std::bind(&MepAnalyzerNode::device_info_callback, this, std::placeholders::_1),
    [&]() {
      rclcpp::SubscriptionOptions opts;
      opts.callback_group = callback_group;
      return opts;
    }());

  const auto qos = rclcpp::QoS(rclcpp::KeepLast(65535))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);

  eeg_subscriber = this->create_subscription<eeg_interfaces::msg::Sample>(
    EEG_RAW_TOPIC.c_str(),
    qos,
    std::bind(&MepAnalyzerNode::eeg_sample_callback, this, std::placeholders::_1),
    [&]() {
      rclcpp::SubscriptionOptions opts;
      opts.callback_group = callback_group;
      return opts;
    }());

  analyze_mep_service = this->create_service<mep_interfaces::srv::AnalyzeMep>(
    SERVICE_ANALYZE_MEP.c_str(),
    std::bind(&MepAnalyzerNode::analyze_mep_handler, this, std::placeholders::_1, std::placeholders::_2),
    rmw_qos_profile_services_default,
    callback_group);
}

void MepAnalyzerNode::device_info_callback(const eeg_interfaces::msg::EegDeviceInfo::SharedPtr msg)
{
  if (msg->sampling_frequency == 0u) {
    return;
  }

  std::lock_guard<std::mutex> lock(buffer_mutex);

  sampling_frequency_hz = msg->sampling_frequency;

  if (eeg_buffer.max_size() == 0) {
    const double fs_hz = static_cast<double>(*sampling_frequency_hz);
    const size_t capacity = static_cast<size_t>(std::ceil(eeg_bufferWINDOW_S * fs_hz));
    eeg_buffer.reset(std::max<size_t>(capacity, 1));
  }

  buffer_cv.notify_all();
}

void MepAnalyzerNode::eeg_sample_callback(const eeg_interfaces::msg::Sample::SharedPtr msg)
{
  std::lock_guard<std::mutex> lock(buffer_mutex);

  if (eeg_buffer.max_size() == 0) {
    // Wait for device info to initialize the buffer.
    return;
  }

  if (last_sample_index.has_value()) {
    const uint64_t expected = *last_sample_index + 1;
    if (msg->sample_index != expected) {
      // Non-contiguous sample indices indicate a gap (drop) or reset.
      samples_dropped_counter++;
    }
  }
  last_sample_index = msg->sample_index;

  if (msg->trigger_a) {
    last_trigger_a_time_s = msg->time;
  }

  eeg_buffer.append(*msg);
  buffer_cv.notify_all();
}

bool MepAnalyzerNode::wait_for_next_stimulation_time(double & stimulation_time_s)
{
  std::optional<double> baseline_trigger;
  {
    std::lock_guard<std::mutex> lock(buffer_mutex);
    baseline_trigger = last_trigger_a_time_s;
  }

  while (rclcpp::ok()) {
    std::unique_lock<std::mutex> lock(buffer_mutex);
    buffer_cv.wait_for(lock, std::chrono::duration<double>(WAIT_POLL_PERIOD_S));

    if (last_trigger_a_time_s.has_value() &&
        (!baseline_trigger.has_value() || *last_trigger_a_time_s > *baseline_trigger)) {
      stimulation_time_s = *last_trigger_a_time_s;
      return true;
    }
  }
  return false;
}

bool MepAnalyzerNode::wait_until_buffer_covers(double start_time_s, double end_time_s)
{
  while (rclcpp::ok()) {
    std::unique_lock<std::mutex> lock(buffer_mutex);
    buffer_cv.wait_for(lock, std::chrono::duration<double>(WAIT_POLL_PERIOD_S));

    if (eeg_buffer.empty()) {
      continue;
    }

    const double oldest_t = eeg_buffer.oldest().time;
    const double newest_t = eeg_buffer.newest().time;

    if (oldest_t > start_time_s) {
      return false;  // LATE
    }

    if (newest_t >= end_time_s) {
      return true;
    }
  }
  return false;
}

bool MepAnalyzerNode::extract_emg_window(
  uint8_t emg_channel,
  double start_time_s,
  double end_time_s,
  std::vector<double> & out_emg) const
{
  out_emg.clear();
  if (eeg_buffer.empty()) {
    return false;
  }

  for (size_t i = 0; i < eeg_buffer.size(); ++i) {
    const auto & s = eeg_buffer.at(i);
    if (s.time < start_time_s) {
      continue;
    }
    if (s.time > end_time_s) {
      break;
    }
    if (emg_channel >= s.emg.size()) {
      return false;
    }
    out_emg.push_back(s.emg[emg_channel]);
  }
  return !out_emg.empty();
}

double MepAnalyzerNode::max_minus_min(const std::vector<double> & v)
{
  if (v.empty()) {
    return 0.0;
  }
  auto [mn_it, mx_it] = std::minmax_element(v.begin(), v.end());
  return *mx_it - *mn_it;
}

void MepAnalyzerNode::analyze_mep_handler(
  const std::shared_ptr<mep_interfaces::srv::AnalyzeMep::Request> request,
  std::shared_ptr<mep_interfaces::srv::AnalyzeMep::Response> response)
{
  response->preactivation_passed = true;
  response->amplitude = 0.0;
  response->latency = 0.0;
  response->emg_buffer.clear();
  response->status = mep_interfaces::srv::AnalyzeMep::Response::NO_ERROR;

  double stimulation_time_s = 0.0;
  if (!wait_for_next_stimulation_time(stimulation_time_s)) {
    response->status = mep_interfaces::srv::AnalyzeMep::Response::SAMPLES_DROPPED;
    return;
  }

  const double mep_start = stimulation_time_s + request->mep_time_window_start;
  const double mep_end = stimulation_time_s + request->mep_time_window_end;

  double required_start = mep_start;
  double required_end = mep_end;

  const bool preactivation_enabled = request->preactivation_check_enabled;
  const double pre_start = stimulation_time_s + request->preactivation_check_time_window_start;
  const double pre_end = stimulation_time_s + request->preactivation_check_time_window_end;
  if (preactivation_enabled) {
    required_start = std::min(required_start, std::min(pre_start, pre_end));
    required_end = std::max(required_end, std::max(pre_start, pre_end));
  }

  const uint64_t dropped_before = samples_dropped_counter;

  if (!wait_until_buffer_covers(required_start, required_end)) {
    response->status = mep_interfaces::srv::AnalyzeMep::Response::LATE;
    return;
  }

  // From this point on, work on a stable snapshot.
  std::vector<eeg_interfaces::msg::Sample> snapshot;
  {
    std::lock_guard<std::mutex> lock(buffer_mutex);
    snapshot.reserve(eeg_buffer.size());
    for (size_t i = 0; i < eeg_buffer.size(); ++i) {
      snapshot.push_back(eeg_buffer.at(i));
    }
  }

  if (snapshot.empty()) {
    response->status = mep_interfaces::srv::AnalyzeMep::Response::LATE;
    return;
  }

  // Validate EMG channel against first sample in snapshot.
  if (request->emg_channel >= snapshot.front().emg.size()) {
    response->status = mep_interfaces::srv::AnalyzeMep::Response::INVALID_EMG_CHANNEL;
    return;
  }

  // Helper to extract from snapshot.
  auto extract_window_from_snapshot = [&](double start_t, double end_t, std::vector<double> & out) -> bool {
    out.clear();
    for (const auto & s : snapshot) {
      if (s.time < start_t) {
        continue;
      }
      if (s.time > end_t) {
        break;
      }
      out.push_back(s.emg[request->emg_channel]);
    }
    return !out.empty();
  };

  if (preactivation_enabled) {
    std::vector<double> pre;
    if (!extract_window_from_snapshot(std::min(pre_start, pre_end), std::max(pre_start, pre_end), pre)) {
      response->status = mep_interfaces::srv::AnalyzeMep::Response::LATE;
      return;
    }
    const double voltage_range = max_minus_min(pre);
    response->preactivation_passed = voltage_range <= request->preactivation_check_voltage_range_limit;
    if (!response->preactivation_passed) {
      // Keep NO_ERROR status; caller uses preactivation_passed separately.
    }
  }

  std::vector<double> mep;
  if (!extract_window_from_snapshot(std::min(mep_start, mep_end), std::max(mep_start, mep_end), mep)) {
    response->status = mep_interfaces::srv::AnalyzeMep::Response::LATE;
    return;
  }
  response->emg_buffer = mep;

  // Compute amplitude and latency.
  auto max_it = std::max_element(mep.begin(), mep.end());
  auto min_it = std::min_element(mep.begin(), mep.end());
  response->amplitude = *max_it - *min_it;

  // Latency: time of max sample minus stimulation time.
  // Use the timestamp from the snapshot at the max index.
  const size_t max_idx = static_cast<size_t>(std::distance(mep.begin(), max_it));
  double max_time = std::numeric_limits<double>::quiet_NaN();
  {
    size_t seen = 0;
    for (const auto & s : snapshot) {
      if (s.time < std::min(mep_start, mep_end)) {
        continue;
      }
      if (s.time > std::max(mep_start, mep_end)) {
        break;
      }
      if (seen == max_idx) {
        max_time = s.time;
        break;
      }
      seen++;
    }
  }
  if (std::isfinite(max_time)) {
    response->latency = max_time - stimulation_time_s;
  } else {
    response->latency = 0.0;
  }

  const uint64_t dropped_after = samples_dropped_counter;
  if (dropped_after != dropped_before) {
    response->status = mep_interfaces::srv::AnalyzeMep::Response::SAMPLES_DROPPED;
  }
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<MepAnalyzerNode>();

  // Service handler may wait while subscription keeps filling the buffer.
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}

