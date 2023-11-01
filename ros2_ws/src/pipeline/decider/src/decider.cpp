#include <chrono>
#include <filesystem>

#include "decider_wrapper.h"
#include "decider.h"

#include "memory_utils.h"
#include "scheduling_utils.h"

#include "std_msgs/msg/string.hpp"

using namespace std::chrono;
using namespace std::placeholders;

const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";

const std::string PROJECTS_DIRECTORY = "projects/";

/* XXX: Needs to match the values in system_state_bridge.cpp. */
const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL = 20ms;
const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE = 5ms;


EegDecider::EegDecider() : Node("decider") {
  /* Subscriber for EEG info. */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->eeg_info_subscriber = this->create_subscription<eeg_interfaces::msg::EegInfo>(
    EEG_INFO_TOPIC,
    qos_persist_latest,
    std::bind(&EegDecider::update_eeg_info, this, _1));

  /* Subscriber for system state. */
  const auto DEADLINE_NS = std::chrono::nanoseconds(SYSTEM_STATE_PUBLISHING_INTERVAL + SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE);

  auto qos_system_state = rclcpp::QoS(rclcpp::KeepLast(1))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
      .deadline(DEADLINE_NS)
      .lifespan(DEADLINE_NS);

  rclcpp::SubscriptionOptions subscription_options;
  subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "System state not received within deadline.");
  };

  this->system_state_subscriber = this->create_subscription<mtms_device_interfaces::msg::SystemState>(
    "/mtms_device/system_state",
    qos_system_state,
    std::bind(&EegDecider::handle_system_state, this, _1),
    subscription_options);

  /* Subscriber for preprocessed EEG data. */
  auto preprocessed_eeg_subscriber_callback = [this](const std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample> msg) -> void {
    auto start = std::chrono::high_resolution_clock::now();

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Received preprocessed EEG datapoint on topic %s with timestamp %.4f.",
                         EEG_PREPROCESSED_TOPIC.c_str(),
                         msg->time);

    this->process_eeg_sample(msg);

    /* Print the time taken to preprocess the datapoint. */

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    RCLCPP_DEBUG(this->get_logger(),
                 "Time taken to process EEG datapoint: %.3f ms.",
                 1000 * elapsed.count());
  };

  this->preprocessed_eeg_subscriber = create_subscription<eeg_interfaces::msg::PreprocessedEegSample>(
    EEG_PREPROCESSED_TOPIC,
    /* TODO: Should the queue be 1 samples long to make it explicit if we are too slow? */
    5000,
    preprocessed_eeg_subscriber_callback);

  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", EEG_PREPROCESSED_TOPIC.c_str());

  /* Subscriber for EEG trigger. */
  this->eeg_trigger_subscriber = create_subscription<eeg_interfaces::msg::Trigger>(
    "/eeg/trigger",
    10,
    std::bind(&EegDecider::handle_eeg_trigger, this, _1));

  /* Subscriber for active project. */
  this->active_project_subscriber = create_subscription<std_msgs::msg::String>(
    "/projects/active",
    qos_persist_latest,
    std::bind(&EegDecider::set_active_project, this, _1));

  /* Publisher for listing deciders. */
  this->decider_list_publisher = this->create_publisher<project_interfaces::msg::DeciderList>(
    "/pipeline/decider/list",
    qos_persist_latest);

  /* Service for changing decider. */
  this->set_decider_service = this->create_service<project_interfaces::srv::SetDecider>(
    "/pipeline/decider/set",
    std::bind(&EegDecider::set_decider, this, _1, _2));

  /* Service for enabling and disabling decider. */
  this->set_decider_enabled_service = this->create_service<project_interfaces::srv::SetDeciderEnabled>(
    "/pipeline/decider/enabled/set",
    std::bind(&EegDecider::set_decider_enabled, this, _1, _2));

  /* Publisher for latency message. */
  this->latency_publisher = this->create_publisher<pipeline_interfaces::msg::Latency>(
    "/pipeline/latency",
    10);

  /* Publisher for decider enabled message. */
  this->decider_enabled_publisher = this->create_publisher<std_msgs::msg::Bool>(
    "/pipeline/decider/enabled",
    qos_persist_latest);

  /* Publisher for event trigger. */
  this->event_trigger_publisher = this->create_publisher<event_interfaces::msg::EventTrigger>(
    "/event/trigger",
    10);

  /* Subscriber for event trigger readiness. */
  this->ready_for_event_trigger_subscriber = this->create_subscription<event_interfaces::msg::ReadyForEventTrigger>(
    "/event/trigger/ready",
    10,
    std::bind(&EegDecider::update_ready_for_event_trigger, this, _1));

  /* Initialize variables. */
  rclcpp::Logger logger = rclcpp::get_logger("decider_wrapper");
  this->decider_wrapper = std::make_unique<DeciderWrapper>(logger);

  this->sample_buffer = RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample>>();
}

/* Functions to reset the decider state. */
void EegDecider::reset_decider_module() {
  if (this->script_directory == UNSET_STRING ||
      this->module_name == UNSET_STRING ||
      this->num_of_eeg_channels == UNSET_NUM_OF_CHANNELS ||
      this->num_of_emg_channels == UNSET_NUM_OF_CHANNELS ||
      this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {

    return;
  }
  this->decider_wrapper->reset_module(
    this->script_directory,
    this->module_name,
    this->num_of_eeg_channels,
    this->num_of_emg_channels,
    this->sampling_frequency);
}

/* Note that this function can be called even if decider wrapper hasn't been initialized yet, it will just reset
   the sample buffer to 0 elements. */
void EegDecider::reset_sample_buffer() {
  size_t buffer_size = this->decider_wrapper->get_buffer_size();
  this->sample_buffer.reset(buffer_size);

  RCLCPP_DEBUG(this->get_logger(), "Sample buffer reset to %lu elements.", buffer_size);
}

/* System state handler. */
void EegDecider::handle_system_state(const std::shared_ptr<mtms_device_interfaces::msg::SystemState> msg) {
  auto new_session_state = msg->session_state;

  if (this->session_state.value != new_session_state.value) {
    RCLCPP_INFO(this->get_logger(), "Session state changed from %d to %d.",
                this->session_state.value, new_session_state.value);
  }

  /* Stopping a session takes several seconds, whereas if another session is started immediately after the previous
      one is stopped, the mTMS device remains in "stopped" state only for a very short period of time. Hence, check both conditions
      to ensure that we notice if the session is stopped. */
  if (this->session_state.value == mtms_device_interfaces::msg::SessionState::STARTED &&
      (new_session_state.value == mtms_device_interfaces::msg::SessionState::STOPPING ||
       new_session_state.value == mtms_device_interfaces::msg::SessionState::STOPPED)) {

    this->reset_decider_module();
    this->reset_sample_buffer();
  }
  this->session_state = new_session_state;
}

/* Listing and setting EEG deciders. */

void EegDecider::set_decider_enabled(
      const std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Response> response) {

  bool enabled = request->enabled;

  /* Update local state variable. */
  this->decider_enabled = enabled;

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::Bool();
  msg.data = enabled;

  this->decider_enabled_publisher->publish(msg);

  /* Reset sample buffer when enabling decider to avoid using remains of old EEG data. */
  if (enabled) {
    reset_sample_buffer();
  }

  RCLCPP_INFO(this->get_logger(), "%s decider.", enabled ? "Enabling" : "Disabling");

  response->success = true;
}

void EegDecider::set_decider(
      const std::shared_ptr<project_interfaces::srv::SetDecider::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDecider::Response> response) {

  this->module_name = request->decider;

  RCLCPP_INFO(this->get_logger(), "Setting decider to: %s.", this->module_name.c_str());

  /* Reset the wrapper to use the changed decider module. */
  reset_decider_module();

  /* We don't want left-over samples from the previous decider, hence
     reset the sample buffer. */
  reset_sample_buffer();

  response->success = true;
}

void EegDecider::set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  std::ostringstream oss;
  oss << PROJECTS_DIRECTORY << this->active_project << "/decider";
  this->script_directory = oss.str();

  RCLCPP_INFO(this->get_logger(), "Active project set to: %s.", this->active_project.c_str());

  update_decider_list();
}

std::vector<std::string> EegDecider::list_python_scripts(const std::string& path) {
  std::vector<std::string> scripts;

  /* Check that the directory exists. */
  if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
    std::cerr << "Warning: Directory does not exist: " << path << std::endl;
    return scripts;
  }

  /* List all .py files in the directory. */
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".py") {
      scripts.push_back(entry.path().stem().string());
    }
  }
  return scripts;
}

void EegDecider::update_decider_list() {
  auto scripts = this->list_python_scripts(this->script_directory);

  auto msg = project_interfaces::msg::DeciderList();
  msg.scripts = scripts;

  this->decider_list_publisher->publish(msg);
}

/* EEG functions */

void EegDecider::update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg) {
  this->sampling_frequency = msg->sampling_frequency;
  this->num_of_eeg_channels = msg->num_of_eeg_channels;
  this->num_of_emg_channels = msg->num_of_emg_channels;

  this->sampling_period = 1.0 / this->sampling_frequency;

  RCLCPP_INFO(this->get_logger(), " ");
  RCLCPP_INFO(this->get_logger(), "Eeg configuration");
  RCLCPP_INFO(this->get_logger(), " ");
  RCLCPP_INFO(this->get_logger(), "  - Sampling frequency: %d Hz", this->sampling_frequency);
  RCLCPP_INFO(this->get_logger(), "  - # of EEG channels: %d", this->num_of_eeg_channels);
  RCLCPP_INFO(this->get_logger(), "  - # of EMG channels: %d", this->num_of_emg_channels);
  RCLCPP_INFO(this->get_logger(), " ");

  /* The number of EEG and EMG channels may have changed, therefore reset decider Python module. */
  reset_decider_module();

  /* EEG info is updated if streaming is restarted on the EEG device. We don't want
     left-over samples from the previous run, therefore reset the sample buffer. */
  reset_sample_buffer();
}

/* XXX: Very close to a similar check in eeg_gatherer.cpp and other pipeline stages. Unify? */
void EegDecider::check_dropped_samples(double_t sample_time) {
  if (this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {
    RCLCPP_WARN(this->get_logger(), "Sampling frequency not received, cannot check for dropped samples.");
  }

  if (this->sampling_frequency != UNSET_SAMPLING_FREQUENCY &&
      this->previous_time) {

    auto time_diff = sample_time - this->previous_time;
    auto threshold = this->sampling_period + this->TOLERANCE_S;

    if (time_diff > threshold) {
      /* Err if sample(s) were dropped. */
      RCLCPP_ERROR(this->get_logger(),
          "Sample(s) dropped. Time difference between consecutive samples: %.5f, should be: %.5f, limit: %.5f", time_diff, this->sampling_period, threshold);

    } else {
      /* If log-level is set to DEBUG, print time difference for all samples, regardless of if samples were dropped or not. */
      RCLCPP_DEBUG(this->get_logger(),
        "Time difference between consecutive samples: %.5f", time_diff);
    }
  }
  this->previous_time = sample_time;
}

/* Handle EEG trigger, indicating a pulse.

   TODO: See the similar logic in preprocessor.cpp and the disclaimer there; we should rely explicitly either on the pulse feedback
     or the EEG trigger.
*/
void EegDecider::handle_eeg_trigger(const std::shared_ptr<eeg_interfaces::msg::Trigger> msg) {
  double_t trigger_time = msg->time;

  RCLCPP_INFO(this->get_logger(), "Registered EEG trigger at: %.5f (s), interpreting as a pulse.", trigger_time);

  if (!this->decision_times.empty()) {
    double_t sample_time = this->decision_times.front();
    this->decision_times.pop();

    double_t time_difference = trigger_time - sample_time;

    RCLCPP_INFO(this->get_logger(), "Time difference between the decision time and the pulse: %.5f (s)", time_difference);

    auto msg = pipeline_interfaces::msg::Latency();
    msg.latency = time_difference;
    msg.sample_time = sample_time;

    this->latency_publisher->publish(msg);

  } else {
    RCLCPP_ERROR(this->get_logger(), "No previous pulse times found in the queue.");
  }
}

void EegDecider::update_ready_for_event_trigger(const std::shared_ptr<event_interfaces::msg::ReadyForEventTrigger> msg) {
  this->ready_for_event_trigger = true;

  RCLCPP_INFO(this->get_logger(), "Ready for event trigger.");
}

void EegDecider::process_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample> msg) {
  auto start_time = std::chrono::high_resolution_clock::now();

  double_t sample_time = msg->time;

  check_dropped_samples(sample_time);

  if (!this->decider_enabled) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Decider not enabled");
    return;
  }
  if (this->module_name == UNSET_STRING) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Decider enabled but not selected");
    return;
  }
  if (!this->decider_wrapper->is_initialized()) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Decider enabled and selected but not initialized");
    return;
  }

  this->sample_buffer.append(msg);

  if (!this->sample_buffer.is_full()) {
    return;
  }

  auto [send_event_trigger, success] = this->decider_wrapper->process(
    this->sample_buffer,
    sample_time,
    ready_for_event_trigger);

  if (success) {
    /* Measure the processing time of the sample.  TODO: Unused at the moment. */
    auto end_time = std::chrono::high_resolution_clock::now();
    double_t processing_time = std::chrono::duration<double_t>(end_time - start_time).count();

    /* Send event trigger if desired. */
    if (send_event_trigger) {
      this->decision_times.push(sample_time);

      RCLCPP_INFO(this->get_logger(), "Sending event trigger at time %.3f (s).", sample_time);

      auto msg = event_interfaces::msg::EventTrigger();
      this->event_trigger_publisher->publish(msg);

      ready_for_event_trigger = false;
    }

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                          1000,
                          "Processed EEG sample at time %.3f (s).",
                          sample_time);
  } else {
    RCLCPP_ERROR_THROTTLE(this->get_logger(),
                          *this->get_clock(),
                          1000,
                          "Python call failed, not processing EEG sample at time %.3f (s).",
                          sample_time);
  }
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("decider"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegDecider>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("decider"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
