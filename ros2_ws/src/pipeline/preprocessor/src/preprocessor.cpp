#include <chrono>
#include <filesystem>

#include "preprocessor_wrapper.h"
#include "preprocessor.h"

#include "memory_utils.h"
#include "scheduling_utils.h"

#include "std_msgs/msg/string.hpp"

using namespace std::placeholders;

const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";

const std::string PROJECTS_DIRECTORY = "projects/";

EegPreprocessor::EegPreprocessor() : Node("preprocessor") {
  this->eeg_preprocessed_publisher = this->create_publisher<eeg_interfaces::msg::PreprocessedEegSample>(EEG_PREPROCESSED_TOPIC, 5000);

  /* Create subscriber for EEG info. */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
        .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);

  this->eeg_info_subscriber = this->create_subscription<eeg_interfaces::msg::EegInfo>(
    EEG_INFO_TOPIC,
    qos_persist_latest,
    std::bind(&EegPreprocessor::update_eeg_info, this, _1));

  /* Create subscriber for EEG data. */

  auto eeg_raw_subscriber_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) -> void {
    auto start = std::chrono::high_resolution_clock::now();

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Received EEG datapoint on topic %s with timestamp %.4f.",
                         EEG_RAW_TOPIC.c_str(),
                         msg->time);

    this->handle_eeg_sample(msg);

    /* Print the time taken to preprocess the datapoint. */

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    RCLCPP_DEBUG(this->get_logger(),
                 "Time taken to preprocess EEG datapoint: %.3f ms.",
                 1000 * elapsed.count());
  };

  this->eeg_raw_subscriber = create_subscription<eeg_interfaces::msg::EegSample>(
    EEG_RAW_TOPIC,
    /* TODO: Should the queue be 1 samples long to make it explicit if we are too slow? */
    5000,
    eeg_raw_subscriber_callback);

  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", EEG_RAW_TOPIC.c_str());

  /* Create a subscriber for active project. */

  this->active_project_subscriber = create_subscription<std_msgs::msg::String>(
    "/projects/active",
    qos_persist_latest,
    std::bind(&EegPreprocessor::set_active_project, this, _1));

  /* Create a publisher for listing preprocessors. */

  this->preprocessor_list_publisher = this->create_publisher<project_interfaces::msg::PreprocessorList>(
    "/pipeline/preprocessor/list",
    qos_persist_latest);

  /* Create service for changing preprocessor. */
  set_preprocessor_service = this->create_service<project_interfaces::srv::SetPreprocessor>(
    "/pipeline/preprocessor/set",
    std::bind(&EegPreprocessor::set_preprocessor, this, _1, _2));

  /* Initialize variables. */

  this->previous_time = UNSET_PREVIOUS_TIME;
  this->sampling_frequency = UNSET_SAMPLING_FREQUENCY;

  this->preprocessor_wrapper = std::make_unique<PreprocessorWrapper>();

  this->sample_buffer = RingBuffer<std::shared_ptr<eeg_interfaces::msg::EegSample>>();
}

/* Listing and setting EEG preprocessors. */

void EegPreprocessor::set_preprocessor(
      const std::shared_ptr<project_interfaces::srv::SetPreprocessor::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPreprocessor::Response> response) {

  this->module_name = request->preprocessor;

  /* Reset the wrapper to use the changed preprocessor module. */
  this->preprocessor_wrapper->reset_module(this->script_directory, this->module_name);

  /* When preprocessor changes, its buffer size potentially changes, hence
     re-initialize arrays in the wrapper. */
  this->preprocessor_wrapper->initialize_arrays();

  /* We don't want left-over samples from the previous preprocessor, hence
     reset the sample buffer. */
  reset_sample_buffer();

  RCLCPP_INFO(this->get_logger(), "Preprocessor set to %s.", this->module_name.c_str());

  response->success = true;
}

/* Reset sample buffer to the size determined by the Python module. */
void EegPreprocessor::reset_sample_buffer() {
  size_t buffer_size = this->preprocessor_wrapper->get_buffer_size();
  this->sample_buffer.reset(buffer_size);

  RCLCPP_INFO(this->get_logger(), "Sample buffer reset to %lu elements.", buffer_size);
}

void EegPreprocessor::set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  std::ostringstream oss;
  oss << PROJECTS_DIRECTORY << this->active_project << "/preprocessor";
  this->script_directory = oss.str();

  RCLCPP_INFO(this->get_logger(), "Active project set to to %s.", this->active_project.c_str());

  update_preprocessor_list();
}

std::vector<std::string> EegPreprocessor::list_python_scripts(const std::string& path) {
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

void EegPreprocessor::update_preprocessor_list() {
  auto scripts = this->list_python_scripts(this->script_directory);

  auto msg = project_interfaces::msg::PreprocessorList();
  msg.scripts = scripts;

  this->preprocessor_list_publisher->publish(msg);
}

/* EEG functions */

void EegPreprocessor::update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg) {
  this->sampling_frequency = msg->sampling_frequency;
  this->num_of_eeg_channels = msg->num_of_eeg_channels;
  this->num_of_emg_channels = msg->num_of_emg_channels;

  this->sampling_period = 1.0 / this->sampling_frequency;

  RCLCPP_INFO(this->get_logger(), "Sampling frequency updated to %d Hz.", this->sampling_frequency);
  RCLCPP_INFO(this->get_logger(), "# of EEG channels updated to %d.", this->num_of_eeg_channels);
  RCLCPP_INFO(this->get_logger(), "# of EMG channels updated to %d.", this->num_of_emg_channels);

  /* The number of EEG and EMG channels may have changed, therefore
     re-initialize arrays in the wrapper. */
  this->preprocessor_wrapper->initialize_arrays();

  /* EEG info is updated if streaming is restarted on the EEG device. We don't want
     left-over samples from the previous run, therefore reset the sample buffer. */
  reset_sample_buffer();
}

/* XXX: Very close to a similar check in eeg_gatherer.cpp and other pipeline stages. Unify? */
void EegPreprocessor::check_dropped_samples(double_t current_time) {
  if (this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {
    RCLCPP_WARN(rclcpp::get_logger("preprocessor"), "Sampling frequency not received, cannot check for dropped samples.");
  }

  if (this->sampling_frequency != UNSET_SAMPLING_FREQUENCY &&
      this->previous_time) {

    auto time_diff = current_time - this->previous_time;
    auto threshold = this->sampling_period + this->TOLERANCE_S;

    if (time_diff > threshold) {
      /* Err if sample(s) were dropped. */
      RCLCPP_ERROR(rclcpp::get_logger("preprocessor"),
          "Sample(s) dropped. Time difference between consecutive samples: %.5f, should be: %.5f, limit: %.5f", time_diff, this->sampling_period, threshold);

    } else {
      /* If log-level is set to DEBUG, print time difference for all samples, regardless of if samples were dropped or not. */
      RCLCPP_DEBUG(rclcpp::get_logger("preprocessor"),
        "Time difference between consecutive samples: %.5f", time_diff);
    }
  }
  this->previous_time = current_time;
}

void EegPreprocessor::handle_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) {
  auto current_time = msg->time;

  check_dropped_samples(current_time);

  if (!this->preprocessor_wrapper->is_initialized()) {
    RCLCPP_INFO_THROTTLE(this->get_logger(),
                          *this->get_clock(),
                          1000,
                          "No preprocessor selected");
    return;
  }

  this->sample_buffer.append(msg);

  if (!this->sample_buffer.is_full()) {
    return;
  }

  auto preprocessed_sample = this->preprocessor_wrapper->process(
    this->sample_buffer,
    current_time);

  this->eeg_preprocessed_publisher->publish(preprocessed_sample);

  RCLCPP_INFO_THROTTLE(this->get_logger(),
                      *this->get_clock(),
                      1000,

                      "Published preprocessed EEG data on topic %s",
                      EEG_PREPROCESSED_TOPIC.c_str());
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
