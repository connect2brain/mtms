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

const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";
const std::string HEALTHCHECK_TOPIC = "/eeg/decider/healthcheck";

const std::string PROJECTS_DIRECTORY = "projects/";

/* Have a long queue to avoid dropping messages. */
const uint16_t EEG_QUEUE_LENGTH = 65535;

/* XXX: Needs to match the values in session_bridge.cpp. */
const milliseconds SESSION_PUBLISHING_INTERVAL = 20ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 5ms;


EegDecider::EegDecider() : Node("decider"), logger(rclcpp::get_logger("decider")) {
  /* Publisher for healthcheck. */
  this->healthcheck_publisher = this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);

  /* Subscriber for mTMS device healthcheck. */
  this->mtms_device_healthcheck_subscriber = create_subscription<system_interfaces::msg::Healthcheck>(
    "/mtms_device/healthcheck",
    10,
    std::bind(&EegDecider::handle_mtms_device_healthcheck, this, _1));

  /* Subscriber for EEG info. */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->eeg_info_subscriber = this->create_subscription<eeg_interfaces::msg::EegInfo>(
    EEG_INFO_TOPIC,
    qos_persist_latest,
    std::bind(&EegDecider::update_eeg_info, this, _1));

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
  this->preprocessed_eeg_subscriber = create_subscription<eeg_interfaces::msg::PreprocessedEegSample>(
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

  /* Publisher for event trigger. */
  this->event_trigger_publisher = this->create_publisher<event_interfaces::msg::EventTrigger>(
    "/event/trigger",
    10);

  /* Subscriber for event trigger readiness. */
  this->ready_for_event_trigger_subscriber = this->create_subscription<event_interfaces::msg::ReadyForEventTrigger>(
    "/event/trigger/ready",
    10,
    std::bind(&EegDecider::update_ready_for_event_trigger, this, _1));

  /* Subscriber for stimulus feedback. */
  this->stimulus_feedback_subscriber = create_subscription<event_interfaces::msg::StimulusFeedback>(
    "/event/stimulus_feedback",
    10,
    std::bind(&EegDecider::handle_stimulus_feedback, this, _1));

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

  this->sample_buffer = RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample>>();
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

  this->decider_state = WAITING_FOR_ENABLED;
}

EegDecider::~EegDecider() {
  inotify_rm_watch(inotify_descriptor, watch_descriptor);
  close(inotify_descriptor);
}

void EegDecider::publish_healthcheck() {
  auto healthcheck = system_interfaces::msg::Healthcheck();

  switch (this->decider_state) {
    case WAITING_FOR_ENABLED:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::NOT_READY;
      healthcheck.status_message = "Decider not enabled";
      healthcheck.actionable_message = "Please enable the decider.";
      break;

    case READY:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::READY;
      healthcheck.status_message = "Ready";
      healthcheck.actionable_message = "Ready";
      break;

    case SAMPLES_DROPPED:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::ERROR;
      healthcheck.status_message = "Samples dropped";
      healthcheck.actionable_message = "Sample(s) dropped in decider. Please restart the measurement.";
      break;

    case MODULE_ERROR:
      healthcheck.status.value = system_interfaces::msg::HealthcheckStatus::ERROR;
      healthcheck.status_message = "Module error";
      healthcheck.actionable_message = "Decider has encountered an error. Please restart the measurement.";
      break;
  }
  this->healthcheck_publisher->publish(healthcheck);
}

void EegDecider::handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) {
  this->mtms_device_available = msg->status.value == system_interfaces::msg::HealthcheckStatus::READY;
}

/* Functions to re-initialize the decider state. */
void EegDecider::initialize_decider_module() {
  if (!this->enabled ||
      this->script_directory == UNSET_STRING ||
      this->module_name == UNSET_STRING ||
      this->num_of_eeg_channels == UNSET_NUM_OF_CHANNELS ||
      this->num_of_emg_channels == UNSET_NUM_OF_CHANNELS ||
      this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {

    return;
  }
  this->decider_wrapper->initialize_module(
    this->script_directory,
    this->module_name,
    this->num_of_eeg_channels,
    this->num_of_emg_channels,
    this->sampling_frequency);
}

/* Note that this function can be called even if decider wrapper hasn't been initialized yet, it will just reset
   the sample buffer to 0 elements. */
void EegDecider::initialize_sample_buffer() {
  size_t buffer_size = this->decider_wrapper->get_buffer_size();
  this->sample_buffer.reset(buffer_size);

  RCLCPP_DEBUG(this->get_logger(), "Sample buffer initialized to %lu elements and reset.", buffer_size);
}

/* Session handler. */
void EegDecider::handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg) {
  auto new_session_state = msg->state;

  if (this->session_state.value != new_session_state.value) {
    RCLCPP_INFO(this->get_logger(), "Session state changed from %d to %d.",
                this->session_state.value, new_session_state.value);
  }

  /* Stopping a session can take several seconds, whereas if another session is started immediately after the previous
      one is stopped, the system remains in "stopped" state only for a very short period of time. Hence, check both conditions
      to ensure that we notice if the session is stopped. */
  if (this->session_state.value == system_interfaces::msg::SessionState::STARTED &&
      (new_session_state.value == system_interfaces::msg::SessionState::STOPPING ||
       new_session_state.value == system_interfaces::msg::SessionState::STOPPED)) {

    /* XXX: It's not the cleanest way to do it to map 'enabled' to decider state in several places (see similar mapping in
         handle_set_decider_enabled function). These two variables should probably be combined into one. */
    this->decider_state = this->enabled ? READY : WAITING_FOR_ENABLED;

    this->initialize_decider_module();
    this->initialize_sample_buffer();
  }
  this->session_state = new_session_state;
}

/* Listing and setting EEG deciders. */

void EegDecider::handle_set_decider_enabled(
      const std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Response> response) {

  /* Update local state variable. */
  this->enabled = request->enabled;

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::Bool();
  msg.data = this->enabled;

  this->decider_enabled_publisher->publish(msg);

  if (this->enabled) {
    initialize_decider_module();

    /* Re-initialize sample buffer to avoid using remains of old EEG data. */
    initialize_sample_buffer();

  } else {
    /* Reset the state of the existing module so that, e.g., memory reserved by the Python module is freed,
       but do not unset the module. */
    this->decider_wrapper->reset_module_state();
  }
  /* Update decider state. */
  this->decider_state = this->enabled ? READY : WAITING_FOR_ENABLED;

  RCLCPP_INFO(this->get_logger(), "%s decider.", this->enabled ? "Enabling" : "Disabling");

  response->success = true;
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
}

void EegDecider::set_decider_module(const std::string module) {
  this->module_name = module;

  RCLCPP_INFO(this->get_logger(), "Decider set to: %s.", this->module_name.c_str());

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::String();
  msg.data = this->module_name;

  this->decider_module_publisher->publish(msg);

  /* Initialize the wrapper to use the changed decider module. */
  initialize_decider_module();

  /* We don't want left-over samples from the previous decider, hence
     re-initialize the sample buffer. */
  initialize_sample_buffer();
}

void EegDecider::handle_set_decider_module(
      const std::shared_ptr<project_interfaces::srv::SetDeciderModule::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderModule::Response> response) {

  set_decider_module(request->module);
  response->success = true;
}

void EegDecider::handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  this->script_directory = PROJECTS_DIRECTORY + "/" + this->active_project + "/decider";

  RCLCPP_INFO(this->get_logger(), "Active project set to: %s.", this->active_project.c_str());

  update_decider_list();

  if (this->modules.size() > 0) {
    this->set_decider_module(this->modules[0]);
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
        this->initialize_decider_module();
        this->initialize_sample_buffer();
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

  /* The number of EEG and EMG channels may have changed, therefore re-initialize decider Python module. */
  initialize_decider_module();

  /* EEG info is updated if streaming is restarted on the EEG device. We don't want
     left-over samples from the previous run, therefore re-initialize the sample buffer. */
  initialize_sample_buffer();
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
      this->decider_state = SAMPLES_DROPPED;

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

/* Handle stimulus feedback, received directly from the mTMS device. */
void EegDecider::handle_stimulus_feedback(const std::shared_ptr<event_interfaces::msg::StimulusFeedback> msg) {

  RCLCPP_INFO(this->get_logger(), "Registered stimulus feedback at: %.5f (s).", msg->execution_time);
  this->calculate_latency(msg->execution_time);
}

void EegDecider::update_ready_for_event_trigger([[maybe_unused]] const std::shared_ptr<event_interfaces::msg::ReadyForEventTrigger> msg) {
  this->ready_for_event_trigger = true;

  RCLCPP_INFO(this->get_logger(), "Ready for event trigger.");
}

void EegDecider::process_sample(const std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample> msg) {
  auto start_time = std::chrono::high_resolution_clock::now();

  double_t sample_time = msg->time;

  check_dropped_samples(sample_time);

  if (!this->enabled) {
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
  if (this->decider_wrapper->error_occurred()) {
    this->decider_state = MODULE_ERROR;

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "An error occurred in decider module, please re-initialize.");
    return;
  }

  this->sample_buffer.append(msg);

  if (!this->sample_buffer.is_full()) {
    return;
  }

  auto [success, send_event_trigger, send_sensory_stimulus] = this->decider_wrapper->process(
    this->sensory_stimulus,
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

    if (send_sensory_stimulus) {
      RCLCPP_INFO(this->get_logger(), "Sending sensory stimulus at time %.3f (s).", sample_time);

      this->sensory_stimulus_publisher->publish(this->sensory_stimulus);
    }
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
