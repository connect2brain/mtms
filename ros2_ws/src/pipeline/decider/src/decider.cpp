#include <chrono>
#include <filesystem>

#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "decider_wrapper.h"
#include "decider.h"

#include "memory_utils.h"
#include "scheduling_utils.h"

#include "std_msgs/msg/string.hpp"

using namespace std::chrono;
using namespace std::placeholders;

const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";
const std::string HEALTHCHECK_TOPIC = "/eeg/decider/healthcheck";

const std::string PROJECTS_DIRECTORY = "projects/";

const std::string DEFAULT_DECIDER_NAME = "dummy";

/* Have a long queue to avoid dropping messages. */
const uint16_t EEG_QUEUE_LENGTH = 65535;

/* XXX: Needs to match the values in session_bridge.cpp. */
const milliseconds SESSION_PUBLISHING_INTERVAL = 20ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 5ms;


EegDecider::EegDecider() : Node("decider"), logger(rclcpp::get_logger("decider")) {
  /* Read ROS parameter: Minimum interval between consecutive pulses (in seconds). */
  auto minimum_pulse_interval_descriptor = rcl_interfaces::msg::ParameterDescriptor{};
  minimum_pulse_interval_descriptor.description = "The minimum interval between consecutive pulses (in seconds)";
  minimum_pulse_interval_descriptor.type = rcl_interfaces::msg::ParameterType::PARAMETER_DOUBLE;
  /* XXX: Have to provide 0.0 as a default value because the parameter server does not interpret NULL correctly
          when the parameter is a double. */
  this->declare_parameter("minimum-pulse-interval", 0.0, minimum_pulse_interval_descriptor);
  this->get_parameter("minimum-pulse-interval", this->minimum_pulse_interval);

  /* Log the minimum pulse interval. */
  RCLCPP_INFO(this->get_logger(), "Configuration:");
  RCLCPP_INFO(this->get_logger(), "  Minimum pulse interval: %.1f (s)", this->minimum_pulse_interval);
  RCLCPP_INFO(this->get_logger(), " ");

  /* Validate the minimum pulse interval. */
  if (this->minimum_pulse_interval <= 0) {
    RCLCPP_ERROR(this->get_logger(), "Invalid minimum pulse interval: %.1f (s)", this->minimum_pulse_interval);
    exit(1);
  }
  if (this->minimum_pulse_interval < 0.5) {
    RCLCPP_WARN(this->get_logger(), "Note: Minimum pulse interval is very low: %.1f (s)", this->minimum_pulse_interval);
    RCLCPP_WARN(this->get_logger(), " ");
  }

  /* Publisher for healthcheck. */
  this->healthcheck_publisher = this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);

  /* Subscriber for mTMS device healthcheck. */
  this->mtms_device_healthcheck_subscriber = create_subscription<system_interfaces::msg::Healthcheck>(
    "/mtms_device/healthcheck",
    10,
    std::bind(&EegDecider::handle_mtms_device_healthcheck, this, _1));

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
    std::bind(&EegDecider::handle_session, this, _1),
    subscription_options);

  /* Subscriber for preprocessed EEG data. */
  this->preprocessed_eeg_subscriber = create_subscription<eeg_interfaces::msg::PreprocessedSample>(
    EEG_PREPROCESSED_TOPIC,
    /* TODO: Should the queue be 1 samples long to make it explicit if we are too slow? */
    EEG_QUEUE_LENGTH,
    std::bind(&EegDecider::process_sample, this, _1));

  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", EEG_PREPROCESSED_TOPIC.c_str());

  /* Subscriber for EEG trigger. */
  this->eeg_trigger_subscriber = create_subscription<eeg_interfaces::msg::Trigger>(
    "/eeg/trigger",
    10,
    std::bind(&EegDecider::handle_eeg_trigger, this, _1));

  /* Subscriber for active project. */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->active_project_subscriber = create_subscription<std_msgs::msg::String>(
    "/projects/active",
    qos_persist_latest,
    std::bind(&EegDecider::handle_set_active_project, this, _1));

  /* Publisher for listing deciders. */
  this->decider_list_publisher = this->create_publisher<project_interfaces::msg::DeciderList>(
    "/pipeline/decider/list",
    qos_persist_latest);

  /* Service for changing decider module. */
  this->set_decider_module_service = this->create_service<project_interfaces::srv::SetDeciderModule>(
    "/pipeline/decider/module/set",
    std::bind(&EegDecider::handle_set_decider_module, this, _1, _2));

  /* Publisher for decider module. */
  this->decider_module_publisher = this->create_publisher<std_msgs::msg::String>(
    "/pipeline/decider/module",
    qos_persist_latest);

  /* Service for enabling and disabling decider. */
  this->set_decider_enabled_service = this->create_service<project_interfaces::srv::SetDeciderEnabled>(
    "/pipeline/decider/enabled/set",
    std::bind(&EegDecider::handle_set_decider_enabled, this, _1, _2));

  /* Publisher for decider enabled message. */
  this->decider_enabled_publisher = this->create_publisher<std_msgs::msg::Bool>(
    "/pipeline/decider/enabled",
    qos_persist_latest);

  /* Publisher for mTMS device trigger. */
  this->trigger_publisher = this->create_publisher<event_interfaces::msg::EventTrigger>(
    "/mtms_device/trigger",
    10);

  /* Publisher for external trigger. */
  this->external_trigger_publisher = this->create_publisher<event_interfaces::msg::EventTrigger>(
    "/event/trigger",
    10);

  /* Subscriber for event trigger readiness. */
  this->ready_for_trigger_subscriber = this->create_subscription<event_interfaces::msg::ReadyForEventTrigger>(
    "/event/trigger/ready",
    10,
    std::bind(&EegDecider::update_ready_for_trigger, this, _1));

  /* Subscriber for trial feedback. */
  this->trial_feedback_subscriber = create_subscription<experiment_interfaces::msg::TrialFeedback>(
    "/trial/feedback",
    10,
    std::bind(&EegDecider::handle_trial_feedback, this, _1));

  /* Publisher for latency. */
  this->latency_publisher = this->create_publisher<pipeline_interfaces::msg::Latency>(
    "/pipeline/latency",
    10);

  /* Publisher for sensory stimulus. */
  this->sensory_stimulus_publisher = this->create_publisher<pipeline_interfaces::msg::SensoryStimulus>(
    "/pipeline/sensory_stimulus",
    10);

  /* Initialize variables. */
  this->decider_wrapper = std::make_unique<DeciderWrapper>(logger);

  this->sample_buffer = RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedSample>>();
  this->sensory_stimulus = pipeline_interfaces::msg::SensoryStimulus();

  /* Initialize inotify. */
  this->inotify_descriptor = inotify_init();
  if (this->inotify_descriptor == -1) {
      RCLCPP_ERROR(this->get_logger(), "Error initializing inotify");
      exit(1);
  }

  /* Set the inotify descriptor to non-blocking. */
  int flags = fcntl(inotify_descriptor, F_GETFL, 0);
  fcntl(inotify_descriptor, F_SETFL, flags | O_NONBLOCK);

  /* Create a timer callback to poll inotify. */
  this->inotify_timer = this->create_wall_timer(
    std::chrono::milliseconds(100),
    std::bind(&EegDecider::inotify_timer_callback, this));

  this->healthcheck_publisher_timer = this->create_wall_timer(
    std::chrono::milliseconds(500),
    std::bind(&EegDecider::publish_healthcheck, this));
}

EegDecider::~EegDecider() {
  inotify_rm_watch(inotify_descriptor, watch_descriptor);
  close(inotify_descriptor);
}

void EegDecider::publish_healthcheck() {
  auto healthcheck = system_interfaces::msg::Healthcheck();

  switch (this->decider_state) {
    case DeciderState::WAITING_FOR_ENABLED:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::NOT_READY;
      healthcheck.status_message = "Decider not enabled";
      healthcheck.actionable_message = "Please enable the decider.";
      break;

    case DeciderState::READY:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::READY;
      healthcheck.status_message = "Ready";
      healthcheck.actionable_message = "Ready";
      break;

    case DeciderState::SAMPLES_DROPPED:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::ERROR;
      healthcheck.status_message = "Samples dropped";
      healthcheck.actionable_message = "Sample(s) dropped in decider.";
      break;

    case DeciderState::MODULE_ERROR:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::ERROR;
      healthcheck.status_message = "Module error";
      healthcheck.actionable_message = "Decider has encountered an error.";
      break;
  }
  this->healthcheck_publisher->publish(healthcheck);
}

void EegDecider::handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg) {
  if (msg->state.value != system_interfaces::msg::SessionState::STARTED) {
    this->reinitialize = true;

    /* Reset the decider state when the session is stopped. */
    reset_decider_state();
  }
}

void EegDecider::handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) {
  this->mtms_device_available = msg->status.value == system_interfaces::msg::HealthcheckStatus::READY;
}

void EegDecider::update_eeg_info(const eeg_interfaces::msg::PreprocessedSampleMetadata& msg) {
  this->sampling_frequency = msg.sampling_frequency;
  this->num_of_eeg_channels = msg.num_of_eeg_channels;
  this->num_of_emg_channels = msg.num_of_emg_channels;

  this->sampling_period = 1.0 / this->sampling_frequency;
}

void EegDecider::initialize_module() {
  if (this->script_directory == UNSET_STRING ||
      this->module_name == UNSET_STRING) {

    RCLCPP_INFO(this->get_logger(), "Not initializing decider module, module unset.");
    return;
  }

  this->decider_wrapper->initialize_module(
    this->script_directory,
    this->module_name,
    this->num_of_eeg_channels,
    this->num_of_emg_channels,
    this->sampling_frequency);

  if (this->decider_wrapper->get_state() != WrapperState::READY) {
    RCLCPP_INFO(this->get_logger(), "Failed to initialize decider.");
    return;
  }

  size_t buffer_size = this->decider_wrapper->get_buffer_size();
  this->sample_buffer.reset(buffer_size);

  RCLCPP_INFO(this->get_logger(), " ");
  RCLCPP_INFO(this->get_logger(), "Initialized decider with the following parameters:");
  RCLCPP_INFO(this->get_logger(), " ");
  RCLCPP_INFO(this->get_logger(), "  - Sampling frequency: %d Hz", this->sampling_frequency);
  RCLCPP_INFO(this->get_logger(), "  - # of EEG channels: %d", this->num_of_eeg_channels);
  RCLCPP_INFO(this->get_logger(), "  - # of EMG channels: %d", this->num_of_emg_channels);
  RCLCPP_INFO(this->get_logger(), "  - Sample buffer size: %lu", buffer_size);
  RCLCPP_INFO(this->get_logger(), " ");
}

void EegDecider::reset_decider_state() {
  this->decider_state = this->enabled ? DeciderState::READY : DeciderState::WAITING_FOR_ENABLED;
}

/* Listing and setting EEG deciders. */
bool EegDecider::set_decider_enabled(bool enabled) {

  /* Only allow enabling the decider if a module is set. */
  if (enabled && this->module_name == UNSET_STRING) {
    RCLCPP_WARN(this->get_logger(), "Cannot enable decider, no module set.");

    return false;
  }

  /* Update global state variable. */
  this->enabled = enabled;

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::Bool();
  msg.data = enabled;

  this->decider_enabled_publisher->publish(msg);

  /* Re-initialize the module each time the decider is enabled. */
  if (enabled) {
    this->reinitialize = true;
  }

  /* Reset decider state. */
  reset_decider_state();

  RCLCPP_INFO(this->get_logger(), "Decider %s.", this->enabled ? "enabled" : "disabled");

  return true;
}

void EegDecider::handle_set_decider_enabled(
      const std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Response> response) {

  response->success = set_decider_enabled(request->enabled);;
}

void EegDecider::unset_decider_module() {
  this->module_name = UNSET_STRING;

  RCLCPP_INFO(this->get_logger(), "Decider module unset.");

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::String();
  msg.data = this->module_name;

  this->decider_module_publisher->publish(msg);

  /* Reset the Python module state. */
  this->decider_wrapper->reset_module_state();

  /* Disable the decider. */
  set_decider_enabled(false);
}

bool EegDecider::set_decider_module(const std::string module) {
  this->module_name = module;

  RCLCPP_INFO(this->get_logger(), "Decider set to: %s.", this->module_name.c_str());

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::String();
  msg.data = this->module_name;

  this->decider_module_publisher->publish(msg);

  /* Re-initialize the module each time the module is reset. */
  this->reinitialize = true;

  return true;
}

void EegDecider::handle_set_decider_module(
      const std::shared_ptr<project_interfaces::srv::SetDeciderModule::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderModule::Response> response) {

  response->success = set_decider_module(request->module);
}

void EegDecider::handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  this->script_directory = PROJECTS_DIRECTORY + "/" + this->active_project + "/decider";

  RCLCPP_INFO(this->get_logger(), "Active project set to: %s.", this->active_project.c_str());

  update_decider_list();

  if (this->modules.size() > 0) {
    /* Set decider module to the default if available, otherwise use the first listed module. */
    if (std::find(this->modules.begin(), this->modules.end(), DEFAULT_DECIDER_NAME) != this->modules.end()) {
      this->set_decider_module(DEFAULT_DECIDER_NAME);
    } else {
      this->set_decider_module(this->modules[0]);
    }
  } else {
    RCLCPP_WARN(this->get_logger(), "No deciders found in project: %s.", this->active_project.c_str());
    this->unset_decider_module();
  }

  update_inotify_watch();
}

/* Inotify functions */

void EegDecider::update_inotify_watch() {
  /* Remove the old watch. */
  inotify_rm_watch(inotify_descriptor, watch_descriptor);

  /* Add a new watch. */
  watch_descriptor = inotify_add_watch(inotify_descriptor, this->script_directory.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
  if (watch_descriptor == -1) {
      RCLCPP_ERROR(this->get_logger(), "Error adding watch for: %s", this->script_directory.c_str());
      return;
  }
}

void EegDecider::inotify_timer_callback() {
  int length = read(inotify_descriptor, inotify_buffer, 1024);

  if (length < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      /* No events, return early. */
      return;
    } else {
      RCLCPP_ERROR(this->get_logger(), "Error reading inotify");
      return;
    }
  }

  int i = 0;
  while (i < length) {
    struct inotify_event *event = (struct inotify_event *)&inotify_buffer[i];
    if (event->len) {
      std::string event_name = event->name;
      if ((event->mask & IN_MODIFY) &&
          (event_name == this->module_name + ".py")) {

        RCLCPP_INFO(this->get_logger(), "The current module '%s' was modified, re-initializing.", this->module_name.c_str());
        this->reinitialize = true;
      }
      if (event->mask & (IN_CREATE | IN_DELETE)) {
        RCLCPP_INFO(this->get_logger(), "File '%s' created or deleted, updating decider list.", event_name.c_str());
        this->update_decider_list();
      }
    }
    i += sizeof(struct inotify_event) + event->len;
  }
}

std::vector<std::string> EegDecider::list_python_modules(const std::string& path) {
  std::vector<std::string> modules;

  /* Check that the directory exists. */
  if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
    RCLCPP_WARN(this->get_logger(), "Warning: Directory does not exist: %s.", path.c_str());
    return modules;
  }

  /* List all .py files in the directory. */
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".py") {
      modules.push_back(entry.path().stem().string());
    }
  }
  return modules;
}

void EegDecider::update_decider_list() {
  this->modules = this->list_python_modules(this->script_directory);

  auto msg = project_interfaces::msg::DeciderList();
  msg.scripts = this->modules;

  this->decider_list_publisher->publish(msg);
}

/* EEG functions */

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
      this->decider_state = DeciderState::SAMPLES_DROPPED;

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

void EegDecider::calculate_latency(double_t pulse_execution_time) {
  if (!this->decision_times.empty()) {
    double_t sample_time = this->decision_times.front();
    this->decision_times.pop();

    /* Calculate the time difference between the decision time and the pulse execution time. */
    double_t time_difference = pulse_execution_time - sample_time;

    RCLCPP_INFO(this->get_logger(), "Time difference between the decision time and the pulse: %.5f (s)", time_difference);

    /* Publish latency ROS message. */
    auto msg = pipeline_interfaces::msg::Latency();
    msg.latency = time_difference;
    msg.sample_time = sample_time;

    this->latency_publisher->publish(msg);

  } else {
    RCLCPP_ERROR(this->get_logger(), "No previous pulse times found in the queue. Can't calculate latency.");
  }
}

/* Handle EEG trigger, indicating a pulse IF mTMS device is available. If not, ignore the EEG trigger, as we get direct
   feedback about the pulse from the mTMS device. */
void EegDecider::handle_eeg_trigger(const std::shared_ptr<eeg_interfaces::msg::Trigger> msg) {
  if (this->mtms_device_available) {
    RCLCPP_INFO(this->get_logger(), "Received EEG trigger, but mTMS device is available, ignoring.");
    return;
  }
  RCLCPP_INFO(this->get_logger(), "Registered EEG trigger at: %.5f (s), interpreting as a pulse.", msg->time);
  this->calculate_latency(msg->time);
}

/* Handle trial feedback. */

/* XXX: As of Apr 2024, this seems to only work when used in conjunction with the UI, which has a back-end that
        publishes the trial feedback. */
void EegDecider::handle_trial_feedback(const std::shared_ptr<experiment_interfaces::msg::TrialFeedback> msg) {

  RCLCPP_INFO(this->get_logger(), "Registered trial feedback at: %.5f (s).", msg->execution_time);
  this->calculate_latency(msg->execution_time);
}

void EegDecider::update_ready_for_trigger([[maybe_unused]] const std::shared_ptr<event_interfaces::msg::ReadyForEventTrigger> msg) {
  this->ready_for_trigger = true;

  RCLCPP_INFO(this->get_logger(), "Ready for event trigger.");
}

void EegDecider::process_sample(const std::shared_ptr<eeg_interfaces::msg::PreprocessedSample> msg) {
  auto start_time = std::chrono::high_resolution_clock::now();

  double_t sample_time = msg->time;

  /* Log if this is the first sample of the session. */
  if (msg->metadata.first_sample_of_session) {
    RCLCPP_INFO(this->get_logger(), "First sample of session received.");
  }

  /* Update EEG info with every new session OR if this is the first EEG sample received. */
  if (msg->metadata.first_sample_of_session || this->first_sample) {
    update_eeg_info(msg->metadata);

    /* Avoid checking for dropped samples on the first sample. */
    this->previous_time = UNSET_PREVIOUS_TIME;

    this->first_sample = false;
  }

  check_dropped_samples(sample_time);

  /* Check that the decider is enabled. */
  if (!this->enabled) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Decider not enabled");
    return;
  }

  /* Assert that module name is set - we shouldn't otherwise allow to enable the decider. */
  assert(this->module_name != UNSET_STRING);

  if (this->reinitialize ||
      this->decider_wrapper->get_state() == WrapperState::UNINITIALIZED ||
      msg->metadata.first_sample_of_session) {

    initialize_module();
    reset_decider_state();

    this->reinitialize = false;
  }

  /* Check that the decider module has not encountered an error. */
  if (this->decider_wrapper->get_state() == WrapperState::ERROR) {
    this->decider_state = DeciderState::MODULE_ERROR;

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "An error occurred in decider module.");
    return;
  }

  /* Append the sample to the buffer. */
  this->sample_buffer.append(msg);

  if (!this->sample_buffer.is_full()) {
    return;
  }

  /* Process the sample. */
  auto [success, send_trigger, send_external_trigger, send_sensory_stimulus] = this->decider_wrapper->process(
    this->sensory_stimulus,
    this->sample_buffer,
    sample_time,
    ready_for_trigger);

  /* Log and return early if the Python call failed. */
  if (!success) {
    RCLCPP_ERROR_THROTTLE(this->get_logger(),
                          *this->get_clock(),
                          1000,
                          "Python call failed, not processing EEG sample at time %.3f (s).",
                          sample_time);
    return;
  }

  /* Measure the processing time of the sample.  TODO: Unused at the moment. */
  auto end_time = std::chrono::high_resolution_clock::now();
  double_t processing_time = std::chrono::duration<double_t>(end_time - start_time).count();

  /* Check that the minimum pulse interval is respected. */
  if (send_trigger || send_external_trigger) {
    if (this->previous_trigger_time != UNSET_PREVIOUS_TIME &&
        sample_time - this->previous_trigger_time < this->minimum_pulse_interval) {

      RCLCPP_ERROR(this->get_logger(), "Minimum pulse interval (%.1f s) not respected, not sending trigger at time %.3f (s).",
                    this->minimum_pulse_interval,
                    sample_time);
      return;
    }
  }

  /* Send trigger if desired. */
  if (send_trigger) {
    this->decision_times.push(sample_time);

    RCLCPP_INFO(this->get_logger(), "Sending trigger at time %.3f (s).", sample_time);

    auto msg = event_interfaces::msg::EventTrigger();
    this->trigger_publisher->publish(msg);

    /* Update the previous trigger time. */
    this->previous_trigger_time = sample_time;

    ready_for_trigger = false;
  }

  /* Send external trigger if desired. */
  if (send_external_trigger) {
    this->decision_times.push(sample_time);

    RCLCPP_INFO(this->get_logger(), "Sending external trigger at time %.3f (s).", sample_time);

    auto msg = event_interfaces::msg::EventTrigger();
    this->external_trigger_publisher->publish(msg);

    /* Update the previous trigger time. */
    this->previous_trigger_time = sample_time;
  }

  /* Send sensory stimulus if desired. */
  if (send_sensory_stimulus) {
    RCLCPP_INFO(this->get_logger(), "Sending sensory stimulus at time %.3f (s).", sample_time);

    this->sensory_stimulus_publisher->publish(this->sensory_stimulus);
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
