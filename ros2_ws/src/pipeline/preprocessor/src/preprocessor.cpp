#include <chrono>
#include <filesystem>

#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "preprocessor_wrapper.h"
#include "preprocessor.h"

#include "memory_utils.h"
#include "scheduling_utils.h"

#include "std_msgs/msg/string.hpp"

using namespace std::chrono;
using namespace std::placeholders;

const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";
const std::string HEALTHCHECK_TOPIC = "/eeg/preprocessor/healthcheck";

const std::string PROJECTS_DIRECTORY = "projects/";

/* XXX: Needs to match the values in session_bridge.cpp. */
const milliseconds SESSION_PUBLISHING_INTERVAL = 20ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 5ms;


EegPreprocessor::EegPreprocessor() : Node("preprocessor"), logger(rclcpp::get_logger("preprocessor")) {
  /* Publisher for healthcheck. */
  this->healthcheck_publisher = this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);

  /* Publisher for preprocessed EEG data. */
  this->preprocessed_eeg_publisher = this->create_publisher<eeg_interfaces::msg::PreprocessedEegSample>(EEG_PREPROCESSED_TOPIC, 5000);

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
    std::bind(&EegPreprocessor::handle_session, this, _1),
    subscription_options);

  /* Subscriber for EEG info. */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->eeg_info_subscriber = this->create_subscription<eeg_interfaces::msg::EegInfo>(
    EEG_INFO_TOPIC,
    qos_persist_latest,
    std::bind(&EegPreprocessor::update_eeg_info, this, _1));

  /* Subscriber for EEG data. */
  this->raw_eeg_subscriber = create_subscription<eeg_interfaces::msg::EegSample>(
    EEG_RAW_TOPIC,
    /* TODO: Should the queue be 1 samples long to make it explicit if we are too slow? */
    5000,
    std::bind(&EegPreprocessor::process_sample, this, _1));

  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", EEG_RAW_TOPIC.c_str());

  /* Subscriber for EEG trigger. */
  this->eeg_trigger_subscriber = create_subscription<eeg_interfaces::msg::Trigger>(
    "/eeg/trigger",
    10,
    std::bind(&EegPreprocessor::handle_eeg_trigger, this, _1));

  /* Subscriber for pulse feedback. */
  this->pulse_feedback_subscriber = create_subscription<event_interfaces::msg::PulseFeedback>(
    "/event/pulse_feedback",
    10,
    std::bind(&EegPreprocessor::handle_pulse_feedback, this, _1));

  /* Subscriber for active project. */
  this->active_project_subscriber = create_subscription<std_msgs::msg::String>(
    "/projects/active",
    qos_persist_latest,
    std::bind(&EegPreprocessor::handle_set_active_project, this, _1));

  /* Publisher for listing preprocessors. */
  this->preprocessor_list_publisher = this->create_publisher<project_interfaces::msg::PreprocessorList>(
    "/pipeline/preprocessor/list",
    qos_persist_latest);

  /* Service for changing preprocessor module. */
  this->set_preprocessor_module_service = this->create_service<project_interfaces::srv::SetPreprocessorModule>(
    "/pipeline/preprocessor/module/set",
    std::bind(&EegPreprocessor::handle_set_preprocessor_module, this, _1, _2));

  /* Publisher for preprocessor module. */
  this->preprocessor_module_publisher = this->create_publisher<std_msgs::msg::String>(
    "/pipeline/preprocessor/module",
    qos_persist_latest);

  /* Service for enabling and disabling preprocessor. */
  this->set_preprocessor_enabled_service = this->create_service<project_interfaces::srv::SetPreprocessorEnabled>(
    "/pipeline/preprocessor/enabled/set",
    std::bind(&EegPreprocessor::handle_set_preprocessor_enabled, this, _1, _2));

  /* Publisher for preprocessor enabled message. */
  this->preprocessor_enabled_publisher = this->create_publisher<std_msgs::msg::Bool>(
    "/pipeline/preprocessor/enabled",
    qos_persist_latest);

  /* Initialize variables. */
  this->preprocessor_wrapper = std::make_unique<PreprocessorWrapper>(logger);

  this->sample_buffer = RingBuffer<std::shared_ptr<eeg_interfaces::msg::EegSample>>();
  this->preprocessed_sample = eeg_interfaces::msg::PreprocessedEegSample();

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
    std::bind(&EegPreprocessor::inotify_timer_callback, this));

  this->healthcheck_publisher_timer = this->create_wall_timer(
    std::chrono::milliseconds(500),
    std::bind(&EegPreprocessor::publish_healthcheck, this));

  this->update_healthcheck();
}

EegPreprocessor::~EegPreprocessor() {
  inotify_rm_watch(inotify_descriptor, watch_descriptor);
  close(inotify_descriptor);
}

void EegPreprocessor::update_healthcheck() {
  if (this->enabled) {
    this->status = system_interfaces::msg::HealthcheckStatus::READY;
    this->status_message = "Ready";
    this->actionable_message = "Ready";
  } else {
    this->status = system_interfaces::msg::HealthcheckStatus::NOT_READY;
    this->status_message = "EEG preprocessor not enabled";
    this->actionable_message = "Please enable the EEG preprocessor.";
  }
}

void EegPreprocessor::publish_healthcheck() {
  auto healthcheck = system_interfaces::msg::Healthcheck();

  healthcheck.status.value = status;
  healthcheck.status_message = status_message;
  healthcheck.actionable_message = actionable_message;

  this->healthcheck_publisher->publish(healthcheck);
}

/* Functions to re-initialize the preprocessor state. */
void EegPreprocessor::initialize_preprocessor_module() {
  if (!this->enabled ||
      this->script_directory == UNSET_STRING ||
      this->module_name == UNSET_STRING ||
      this->num_of_eeg_channels == UNSET_NUM_OF_CHANNELS ||
      this->num_of_emg_channels == UNSET_NUM_OF_CHANNELS ||
      this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {

    return;
  }
  this->preprocessor_wrapper->initialize_module(
    this->script_directory,
    this->module_name,
    this->num_of_eeg_channels,
    this->num_of_emg_channels,
    this->sampling_frequency);
}

/* Note that this function can be called even if preprocessor wrapper hasn't been initialized yet, it will just reset
   the sample buffer to 0 elements. */
void EegPreprocessor::initialize_sample_buffer() {
  size_t buffer_size = this->preprocessor_wrapper->get_buffer_size();
  this->sample_buffer.reset(buffer_size);

  RCLCPP_DEBUG(this->get_logger(), "Sample buffer initialized to %lu elements and reset.", buffer_size);
}

/* Session handler. */
void EegPreprocessor::handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg) {
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

    this->initialize_preprocessor_module();
    this->initialize_sample_buffer();
  }
  this->session_state = new_session_state;
}

/* Listing and setting EEG preprocessors. */

void EegPreprocessor::handle_set_preprocessor_enabled(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessorEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessorEnabled::Response> response) {

  /* Update local state variable. */
  this->enabled = request->enabled;

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::Bool();
  msg.data = this->enabled;

  this->preprocessor_enabled_publisher->publish(msg);

  if (this->enabled) {
    initialize_preprocessor_module();

    /* Re-initialize sample buffer to avoid using remains of old EEG data. */
    initialize_sample_buffer();
  } else {
    /* Reset the state of the existing module so that, e.g., memory reserved by the Python module is freed,
       but do not unset the module. */
    this->preprocessor_wrapper->reset_module_state();
  }
  update_healthcheck();

  RCLCPP_INFO(this->get_logger(), "%s preprocessor.", this->enabled ? "Enabling" : "Disabling");

  response->success = true;
}

void EegPreprocessor::unset_preprocessor_module() {
  this->module_name = UNSET_STRING;

  RCLCPP_INFO(this->get_logger(), "Preprocessor module unset.");

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::String();
  msg.data = this->module_name;

  this->preprocessor_module_publisher->publish(msg);

  /* Reset the Python module state. */
  this->preprocessor_wrapper->reset_module_state();
}

void EegPreprocessor::set_preprocessor_module(const std::string module) {
  this->module_name = module;

  RCLCPP_INFO(this->get_logger(), "Preprocessor set to: %s.", this->module_name.c_str());

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::String();
  msg.data = this->module_name;

  this->preprocessor_module_publisher->publish(msg);

  /* Initialize the wrapper to use the changed preprocessor module. */
  initialize_preprocessor_module();

  /* We don't want left-over samples from the previous preprocessor, hence
     re-initialize the sample buffer. */
  initialize_sample_buffer();
}

void EegPreprocessor::handle_set_preprocessor_module(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessorModule::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessorModule::Response> response) {

  set_preprocessor_module(request->module);
  response->success = true;
}

void EegPreprocessor::handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  this->script_directory = PROJECTS_DIRECTORY + "/" + this->active_project + "/preprocessor";

  RCLCPP_INFO(this->get_logger(), "Active project set to: %s.", this->active_project.c_str());

  update_preprocessor_list();

  if (this->modules.size() > 0) {
    this->set_preprocessor_module(this->modules[0]);
  } else {
    RCLCPP_WARN(this->get_logger(), "No preprocessors found in project: %s.", this->active_project.c_str());
    this->unset_preprocessor_module();
  }

  update_inotify_watch();
}

/* Inotify functions */

void EegPreprocessor::update_inotify_watch() {
  /* Remove the old watch. */
  inotify_rm_watch(inotify_descriptor, watch_descriptor);

  /* Add a new watch. */
  watch_descriptor = inotify_add_watch(inotify_descriptor, this->script_directory.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
  if (watch_descriptor == -1) {
      RCLCPP_ERROR(this->get_logger(), "Error adding watch for: %s", this->script_directory.c_str());
      return;
  }
}

void EegPreprocessor::inotify_timer_callback() {
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
        this->initialize_preprocessor_module();
        this->initialize_sample_buffer();
      }
      if (event->mask & (IN_CREATE | IN_DELETE)) {
        RCLCPP_INFO(this->get_logger(), "File '%s' created or deleted, updating preprocessor list.", event_name.c_str());
        this->update_preprocessor_list();
      }
    }
    i += sizeof(struct inotify_event) + event->len;
  }
}

std::vector<std::string> EegPreprocessor::list_python_modules(const std::string& path) {
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

void EegPreprocessor::update_preprocessor_list() {
  this->modules = this->list_python_modules(this->script_directory);

  auto msg = project_interfaces::msg::PreprocessorList();
  msg.scripts = this->modules;

  this->preprocessor_list_publisher->publish(msg);
}

/* EEG functions */

void EegPreprocessor::update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg) {
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

  /* The number of EEG and EMG channels may have changed, therefore re-initialize preprocessor Python module. */
  initialize_preprocessor_module();

  /* EEG info is updated if streaming is restarted on the EEG device. We don't want
     left-over samples from the previous run, therefore re-initialize the sample buffer. */
  initialize_sample_buffer();
}

/* XXX: Very close to a similar check in eeg_gatherer.cpp and other pipeline stages. Unify? */
void EegPreprocessor::check_dropped_samples(double_t sample_time) {
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

/* Handle EEG trigger, indicating a pulse, as well as direct pulse feedback from the mTMS device.

   Note: The mTMS device sends a pulse feedback message when a pulse is given, but this does not apply to
   TMS devices in general. Hence, we also handle EEG triggers as pulses, allowing other TMS devices to work
   with the EEG preprocessor.

   The downside to this logic is that when using a mTMS device with concurrent pulse and trigger out, connected to
   the EEG device, we will get an indication of a pulse twice. This is not a problem, as the direct feedback and the
   EEG trigger should arrive approximately at the same time, hence both will usually be cleared from the queue during the same sample -
   if not, we will have two consecutive samples marked as having a pulse, which is not a problem in the current use cases.

   TODO: However, we should probably have a more robust logic here in the long term; most likely, we would need to know explicitly
   which one to use.
*/
void EegPreprocessor::handle_eeg_trigger(const std::shared_ptr<eeg_interfaces::msg::Trigger> msg) {
  double_t trigger_time = msg->time;
  this->pulse_execution_times.push(trigger_time);

  RCLCPP_INFO(this->get_logger(), "Registered EEG trigger at: %.5f (s), interpreting as a pulse.", trigger_time);
}

void EegPreprocessor::handle_pulse_feedback(const std::shared_ptr<event_interfaces::msg::PulseFeedback> msg) {
  double_t execution_time = msg->execution_time;
  this->pulse_execution_times.push(execution_time);

  RCLCPP_INFO(this->get_logger(), "Registered pulse feedback from the mTMS device at: %.5f (s).", execution_time);
}

bool EegPreprocessor::was_pulse_given(double_t sample_time) {
  bool pulse_given = false;

  /* Remove all execution times from the queue that have happened before the current sample time. */
  while (!pulse_execution_times.empty() && sample_time >= pulse_execution_times.front()) {
    pulse_execution_times.pop();
    pulse_given = true;
  }
  return pulse_given;
}

void EegPreprocessor::process_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) {
  auto start_time = std::chrono::high_resolution_clock::now();

  double_t sample_time = msg->time;

  check_dropped_samples(sample_time);

  bool pulse_given = was_pulse_given(sample_time);

  if (!this->enabled) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Preprocessor not enabled");
    return;
  }
  if (this->module_name == UNSET_STRING) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Preprocessor enabled but not selected");
    return;
  }
  if (!this->preprocessor_wrapper->is_initialized()) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Preprocessor enabled and selected but not initialized");
    return;
  }
  if (this->preprocessor_wrapper->error_occurred()) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "An error occurred in preprocessor module, please re-initialize.");
    return;
  }

  this->sample_buffer.append(msg);

  if (!this->sample_buffer.is_full()) {
    return;
  }

  bool success = this->preprocessor_wrapper->process(
    preprocessed_sample,
    sample_buffer,
    sample_time,
    pulse_given);

  if (success) {
    /* Measure and store the processing time for the sample. */
    auto end_time = std::chrono::high_resolution_clock::now();
    double_t processing_time = std::chrono::duration<double_t>(end_time - start_time).count();

    preprocessed_sample.processing_time = processing_time;

    /* Publish the preprocessed sample. */
    this->preprocessed_eeg_publisher->publish(preprocessed_sample);
  } else {
    RCLCPP_ERROR_THROTTLE(this->get_logger(),
                          *this->get_clock(),
                          1000,
                          "Python call failed, not publishing data on topic %s",
                          EEG_PREPROCESSED_TOPIC.c_str());
  }
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("preprocessor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegPreprocessor>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("preprocessor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
