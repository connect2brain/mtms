#include "mep_analyzer.h"

#include "std_msgs/msg/empty.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cinttypes>
#include <limits>
#include <thread>

using namespace std::chrono_literals;

const std::string MepAnalyzerNode::EEG_RAW_TOPIC = "/mtms/eeg/raw";
const std::string MepAnalyzerNode::EEG_DEVICE_INFO_TOPIC = "/mtms/eeg_device/info";
const std::string MepAnalyzerNode::SERVICE_ANALYZE_MEP = "/mtms/mep/analyze";
const std::string MepAnalyzerNode::SERVICE_GET_TRIGGER_WINDOW = "/mtms/mep/get_trigger_window";
const std::string HEARTBEAT_TOPIC = "/mtms/mep_analyzer/heartbeat";
constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

MepAnalyzerNode::MepAnalyzerNode(const rclcpp::NodeOptions & options)
: Node("mep_analyzer", options)
{
  data_callback_group = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
  service_callback_group = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

  const auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  device_info_subscriber = this->create_subscription<mtms_eeg_interfaces::msg::EegDeviceInfo>(
    EEG_DEVICE_INFO_TOPIC.c_str(),
    qos_persist_latest,
    std::bind(&MepAnalyzerNode::device_info_callback, this, std::placeholders::_1),
    [&]() {
      rclcpp::SubscriptionOptions opts;
      opts.callback_group = data_callback_group;
      return opts;
    }());

  const auto qos = rclcpp::QoS(rclcpp::KeepLast(65535))
    .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
    .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);

  eeg_subscriber = this->create_subscription<mtms_eeg_interfaces::msg::Sample>(
    EEG_RAW_TOPIC.c_str(),
    qos,
    std::bind(&MepAnalyzerNode::eeg_sample_callback, this, std::placeholders::_1),
    [&]() {
      rclcpp::SubscriptionOptions opts;
      opts.callback_group = data_callback_group;
      return opts;
    }());

  analyze_mep_service = this->create_service<mtms_mep_interfaces::srv::AnalyzeMep>(
    SERVICE_ANALYZE_MEP.c_str(),
    std::bind(&MepAnalyzerNode::analyze_mep_handler, this, std::placeholders::_1, std::placeholders::_2),
    rmw_qos_profile_services_default,
    service_callback_group);

  get_trigger_window_service = this->create_service<mtms_mep_interfaces::srv::GetTriggerWindow>(
    SERVICE_GET_TRIGGER_WINDOW.c_str(),
    std::bind(&MepAnalyzerNode::get_trigger_window_handler, this, std::placeholders::_1, std::placeholders::_2),
    rmw_qos_profile_services_default,
    service_callback_group);

  auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
  heartbeat_timer = this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
    heartbeat_publisher->publish(std_msgs::msg::Empty());
  });
}

void MepAnalyzerNode::device_info_callback(const mtms_eeg_interfaces::msg::EegDeviceInfo::SharedPtr msg)
{
  if (msg->sampling_frequency == 0u) {
    return;
  }

  std::lock_guard<std::mutex> lock(buffer_mutex);

  sampling_frequency_hz = msg->sampling_frequency;

  if (eeg_buffer.max_size() == 0) {
    const double fs_hz = static_cast<double>(*sampling_frequency_hz);
    const size_t capacity = static_cast<size_t>(std::ceil(EEG_BUFFER_WINDOW_S * fs_hz));
    eeg_buffer.reset(std::max<size_t>(capacity, 1));
    RCLCPP_INFO(this->get_logger(), "Initialized EEG buffer: sampling_frequency=%.3fHz capacity=%zu window=%.3fs", fs_hz, eeg_buffer.max_size(), EEG_BUFFER_WINDOW_S);
  }

  buffer_cv.notify_all();
}

void MepAnalyzerNode::eeg_sample_callback(const mtms_eeg_interfaces::msg::Sample::SharedPtr msg)
{
  std::lock_guard<std::mutex> lock(buffer_mutex);

  if (eeg_buffer.max_size() == 0) {
    // Wait for device info to initialize the buffer.
    return;
  }

  if (last_sample_index.has_value()) {
    const uint64_t expected = *last_sample_index + 1;
    if (msg->sample_index != expected) {
      RCLCPP_WARN(this->get_logger(), "Sample index mismatch: expected=%" PRIu64 " got=%" PRIu64, expected, msg->sample_index);

      // Non-contiguous sample indices indicate a gap (drop) or reset.
      samples_dropped_counter++;
    }
  }
  last_sample_index = msg->sample_index;

  if (msg->trigger_a) {
    last_trigger_a_time_s = msg->time;
    RCLCPP_INFO(this->get_logger(), "Trigger_a received: time=%.6f", msg->time);
  }

  eeg_buffer.append(*msg);
  buffer_cv.notify_all();
}

bool MepAnalyzerNode::wait_for_next_stimulation_time(double & stimulation_time_s, double timeout_s)
{
  std::optional<double> baseline_trigger;
  {
    std::lock_guard<std::mutex> lock(buffer_mutex);
    baseline_trigger = last_trigger_a_time_s;
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_s);
  while (rclcpp::ok()) {
    std::unique_lock<std::mutex> lock(buffer_mutex);
    const auto wait_status = buffer_cv.wait_until(lock, deadline);

    if (last_trigger_a_time_s.has_value() &&
        (!baseline_trigger.has_value() || *last_trigger_a_time_s > *baseline_trigger)) {
      stimulation_time_s = *last_trigger_a_time_s;
      return true;
    }

    if (wait_status == std::cv_status::timeout) {
      return false;
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
  const std::shared_ptr<mtms_mep_interfaces::srv::AnalyzeMep::Request> request,
  std::shared_ptr<mtms_mep_interfaces::srv::AnalyzeMep::Response> response)
{
  RCLCPP_INFO(this->get_logger(), "analyze_mep_handler called: emg_channel=%u mep_window=[%.3f,%.3f] preactivation_enabled=%s pre_window=[%.3f,%.3f] pre_range_limit=%.3f", static_cast<unsigned>(request->emg_channel), request->mep_time_window_start, request->mep_time_window_end, request->preactivation_check_enabled ? "true" : "false", request->preactivation_check_time_window_start, request->preactivation_check_time_window_end, request->preactivation_check_voltage_range_limit);

  bool preactivation_passed = true;
  response->amplitude = 0.0;
  response->latency = 0.0;
  response->emg_buffer.clear();
  response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::NO_ERROR;

  double stimulation_time_s = 0.0;
  if (!wait_for_next_stimulation_time(stimulation_time_s, ANALYZE_MEP_TIMEOUT_S)) {
    response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::TIMEOUT;
    RCLCPP_WARN(this->get_logger(), "analyze_mep_handler timed out waiting for next trigger: status=%d", response->status);
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
    response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::LATE;
    RCLCPP_WARN(this->get_logger(), "analyze_mep_handler failed to get required EEG coverage: status=%d required=[%.3f,%.3f]", response->status, required_start, required_end);
    return;
  }

  // From this point on, work on a stable snapshot.
  std::vector<mtms_eeg_interfaces::msg::Sample> snapshot;
  {
    std::lock_guard<std::mutex> lock(buffer_mutex);
    snapshot.reserve(eeg_buffer.size());
    for (size_t i = 0; i < eeg_buffer.size(); ++i) {
      snapshot.push_back(eeg_buffer.at(i));
    }
  }

  if (snapshot.empty()) {
    response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::LATE;
    RCLCPP_WARN(this->get_logger(), "analyze_mep_handler snapshot is empty: status=%d", response->status);
    return;
  }

  RCLCPP_INFO(this->get_logger(), "analyze_mep_handler snapshot ready: samples=%zu time_range=[%.3f,%.3f]", snapshot.size(), snapshot.front().time, snapshot.back().time);

  // Validate EMG channel against first sample in snapshot.
  if (request->emg_channel >= snapshot.front().emg.size()) {
    response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::INVALID_EMG_CHANNEL;
    RCLCPP_WARN(this->get_logger(), "analyze_mep_handler invalid emg_channel=%u (snapshot emg.size()=%zu): status=%d", static_cast<unsigned>(request->emg_channel), snapshot.front().emg.size(), response->status);
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
      response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::LATE;
      return;
    }
    const double voltage_range = max_minus_min(pre);
    preactivation_passed = voltage_range <= request->preactivation_check_voltage_range_limit;
    if (!preactivation_passed) {
      response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::PREACTIVATION_FAILED;
    }
  }

  std::vector<double> mep;
  if (!extract_window_from_snapshot(std::min(mep_start, mep_end), std::max(mep_start, mep_end), mep)) {
    response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::LATE;
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
    response->status = mtms_mep_interfaces::srv::AnalyzeMep::Response::SAMPLES_DROPPED;
  }

  RCLCPP_INFO(this->get_logger(), "analyze_mep_handler finished: amplitude=%.6f latency=%.6f preactivation_passed=%s status=%d dropped_before=%" PRIu64 " dropped_after=%" PRIu64, response->amplitude, response->latency, preactivation_passed ? "true" : "false", response->status, dropped_before, dropped_after);
}

void MepAnalyzerNode::get_trigger_window_handler(
  const std::shared_ptr<mtms_mep_interfaces::srv::GetTriggerWindow::Request> request,
  std::shared_ptr<mtms_mep_interfaces::srv::GetTriggerWindow::Response> response)
{
  RCLCPP_INFO(this->get_logger(), "get_trigger_window_handler called: window=[%.3f,%.3f]",
    request->window_start, request->window_end);

  response->eeg_buffer.clear();
  response->emg_buffer.clear();
  response->sampling_frequency = 0;
  response->trigger_index = 0;
  response->status = mtms_mep_interfaces::srv::GetTriggerWindow::Response::NO_ERROR;

  double stimulation_time_s = 0.0;
  if (!wait_for_next_stimulation_time(stimulation_time_s, ANALYZE_MEP_TIMEOUT_S)) {
    response->status = mtms_mep_interfaces::srv::GetTriggerWindow::Response::TIMEOUT;
    RCLCPP_WARN(this->get_logger(), "get_trigger_window_handler timed out waiting for trigger: status=%d", response->status);
    return;
  }

  const double win_start = stimulation_time_s + request->window_start;
  const double win_end   = stimulation_time_s + request->window_end;

  const uint64_t dropped_before = samples_dropped_counter;

  if (!wait_until_buffer_covers(win_start, win_end)) {
    response->status = mtms_mep_interfaces::srv::GetTriggerWindow::Response::LATE;
    RCLCPP_WARN(this->get_logger(), "get_trigger_window_handler failed to get required EEG coverage: status=%d window=[%.3f,%.3f]", response->status, win_start, win_end);
    return;
  }

  // Stable snapshot under lock.
  std::vector<mtms_eeg_interfaces::msg::Sample> snapshot;
  uint32_t fs_hz = 0;
  {
    std::lock_guard<std::mutex> lock(buffer_mutex);
    snapshot.reserve(eeg_buffer.size());
    for (size_t i = 0; i < eeg_buffer.size(); ++i) {
      snapshot.push_back(eeg_buffer.at(i));
    }
    if (sampling_frequency_hz.has_value()) {
      fs_hz = *sampling_frequency_hz;
    }
  }

  if (snapshot.empty()) {
    response->status = mtms_mep_interfaces::srv::GetTriggerWindow::Response::LATE;
    return;
  }

  response->sampling_frequency = fs_hz;

  // Extract samples in [win_start, win_end] and build interleaved channel buffers.
  // Layout: [t0_ch0, t0_ch1, ..., t1_ch0, t1_ch1, ...] (time-major).
  uint32_t trigger_index = 0;
  bool trigger_index_set = false;
  bool in_window = false;
  for (const auto & s : snapshot) {
    if (s.time < win_start) {
      continue;
    }
    if (s.time > win_end) {
      break;
    }
    in_window = true;

    // Count samples before the stimulation to find trigger_index.
    if (!trigger_index_set && s.time >= stimulation_time_s) {
      trigger_index_set = true;
    }
    if (!trigger_index_set) {
      trigger_index++;
    }

    for (const double v : s.eeg) {
      response->eeg_buffer.push_back(v);
    }
    for (const double v : s.emg) {
      response->emg_buffer.push_back(v);
    }
  }

  if (!in_window) {
    response->status = mtms_mep_interfaces::srv::GetTriggerWindow::Response::LATE;
    return;
  }

  response->trigger_index = trigger_index;

  const uint64_t dropped_after = samples_dropped_counter;
  if (dropped_after != dropped_before) {
    response->status = mtms_mep_interfaces::srv::GetTriggerWindow::Response::SAMPLES_DROPPED;
  }

  RCLCPP_INFO(this->get_logger(),
    "get_trigger_window_handler finished: eeg_buffer=%zu emg_buffer=%zu trigger_index=%u fs=%u status=%d",
    response->eeg_buffer.size(), response->emg_buffer.size(), response->trigger_index,
    response->sampling_frequency, response->status);
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

