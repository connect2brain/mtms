#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

#include "eeg_recorder.h"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::placeholders;

const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";

const std::string PROJECTS_DIRECTORY = "projects";
const std::string PREPROCESSED_EEG_DATA_SUBDIRECTORY = "/eeg_recorder/preprocessed";
const std::string RAW_EEG_DATA_SUBDIRECTORY = "/eeg_recorder/raw";

const milliseconds SESSION_PUBLISHING_INTERVAL = 1ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 2ms;

/* Have a queue size of 5000 to avoid dropping messages - corresponds to 1 s of data with 5 kHz sampling frequency. */
const uint16_t QUEUE_SIZE = 5000;

const uint16_t BATCH_SIZE_IN_SAMPLES = 5000;

EegRecorder::EegRecorder() : Node("eeg_recorder") {
  /* Subscriber for active project. */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->active_project_subscriber = create_subscription<std_msgs::msg::String>(
    "/projects/active",
    qos_persist_latest,
    std::bind(&EegRecorder::handle_set_active_project, this, _1));

  /* QOS for session */
  const auto DEADLINE_NS = std::chrono::nanoseconds(SESSION_PUBLISHING_INTERVAL + SESSION_PUBLISHING_INTERVAL_TOLERANCE);

  auto qos_session = rclcpp::QoS(rclcpp::KeepLast(1))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
      .deadline(DEADLINE_NS)
      .lifespan(DEADLINE_NS);

  /* Subscriber for session. */
  this->session_subscriber = create_subscription<system_interfaces::msg::Session>(
    "/system/session",
    qos_session,
    std::bind(&EegRecorder::handle_session, this, _1));

  /* Subscriber for raw EEG. */
  eeg_raw_subscriber = this->create_subscription<eeg_interfaces::msg::EegSample>(
    EEG_RAW_TOPIC,
    QUEUE_SIZE,
    std::bind(&EegRecorder::handle_raw_eeg_sample, this, _1));

  /* Subscriber for preprocessed EEG. */
  eeg_preprocessed_subscriber = this->create_subscription<eeg_interfaces::msg::PreprocessedEegSample>(
    EEG_PREPROCESSED_TOPIC,
    QUEUE_SIZE,
    std::bind(&EegRecorder::handle_preprocessed_eeg_sample, this, _1));
}

void EegRecorder::handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  this->raw_data_directory = PROJECTS_DIRECTORY + "/" + this->active_project + "/" + RAW_EEG_DATA_SUBDIRECTORY;
  this->preprocessed_data_directory = PROJECTS_DIRECTORY + "/" + this->active_project + "/" + PREPROCESSED_EEG_DATA_SUBDIRECTORY;

  RCLCPP_INFO(this->get_logger(), "Active project set to: %s.", this->active_project.c_str());
}

void EegRecorder::handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg) {
  if (this->active_project.empty()) {
    RCLCPP_WARN_THROTTLE(this->get_logger(),
                         *this->get_clock(), 5000,
                         "No active project set.");
    return;
  }

  auto session_state = msg->state.value;

  if (session_state == system_interfaces::msg::SessionState::STARTED &&
      current_session_state != system_interfaces::msg::SessionState::STARTED) {

    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);

    std::ostringstream timestamp_stream;
    timestamp_stream << std::put_time(std::localtime(&now_time), "%Y-%m-%d-%H-%M-%S");

    /* TODO: Experiment name and subject name are empty strings for now, make them configurable via UI. */
    if (experiment_name != "" && subject_name != "") {
      filename = timestamp_stream.str() + "_" + experiment_name + "_" + subject_name + ".csv";
    } else {
      filename = timestamp_stream.str() + ".csv";
    }
    raw_file_path = raw_data_directory + "/" + filename;
    preprocessed_file_path = preprocessed_data_directory + "/" + filename;

    RCLCPP_INFO(this->get_logger(), "Recording session into file: %s", filename.c_str());
  }

  if (session_state != system_interfaces::msg::SessionState::STARTED &&
      current_session_state == system_interfaces::msg::SessionState::STARTED) {

    RCLCPP_INFO(this->get_logger(), "Session stopped.", filename.c_str());
    if (raw_file.is_open()) {
      write_raw_buffer();
      raw_file.close();
    }
    if (preprocessed_file.is_open()) {
      write_preprocessed_buffer();
      preprocessed_file.close();
    }
  }
  current_session_state = session_state;
}

void EegRecorder::write_raw_buffer() {
  /* Write to file. */
  raw_file << raw_buffer.str();
  raw_file.flush();

  /* Clear the buffer. */
  raw_buffer.str("");
  raw_buffer.clear();
}

void EegRecorder::handle_raw_eeg_sample([[maybe_unused]] const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) {
  /* TODO: Duplicating the code from handle_preprocessed_eeg_sample function for now. Later, unify if it turns out that
       they did not diverge too much. */

  if (!raw_file.is_open()) {
    raw_file.open(raw_file_path);

    /* Check if file was opened successfully. */
    if (!raw_file.is_open()) {
      RCLCPP_ERROR(this->get_logger(), "Error opening file in path: %s", raw_file_path.c_str());
      return;
    }
  }

  raw_buffer << std::fixed << std::setprecision(4) << msg->time;

  /* Helper function to concatenate with comma. */
  auto comma_join = [](const std::string &accum, double_t value) {
      return accum.empty() ? std::to_string(value) : accum + "," + std::to_string(value);
  };

  /* Add eeg_data if available. */
  if (!msg->eeg_data.empty()) {
      raw_buffer << ",";
      std::string eeg_str = std::accumulate(std::begin(msg->eeg_data), std::end(msg->eeg_data), std::string{}, comma_join);
      raw_buffer << eeg_str;
  }

  /* Add emg_data if available. */
  if (!msg->emg_data.empty()) {
      raw_buffer << ",";
      std::string emg_str = std::accumulate(std::begin(msg->emg_data), std::end(msg->emg_data), std::string{}, comma_join);
      raw_buffer << emg_str;
  }

  raw_buffer << "\n";

  /* Update sample count. */
  raw_sample_count++;

  if (raw_sample_count % BATCH_SIZE_IN_SAMPLES == 0) {
    write_raw_buffer();
  }
}

void EegRecorder::write_preprocessed_buffer() {
  /* Write to file. */
  preprocessed_file << preprocessed_buffer.str();
  preprocessed_file.flush();

  /* Clear the buffer. */
  preprocessed_buffer.str("");
  preprocessed_buffer.clear();
}

void EegRecorder::handle_preprocessed_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample> msg) {
  if (!preprocessed_file.is_open()) {
    preprocessed_file.open(preprocessed_file_path);

    /* Check if file was opened successfully. */
    if (!preprocessed_file.is_open()) {
      RCLCPP_ERROR(this->get_logger(), "Error opening file in path: %s", preprocessed_file_path.c_str());
      return;
    }
  }

  preprocessed_buffer << std::fixed << std::setprecision(4) << msg->time << "," << msg->valid << "," << std::setprecision(6) << msg->processing_time << std::setprecision(4);

  /* Helper function to concatenate with comma. */
  auto comma_join = [](const std::string &accum, double_t value) {
      return accum.empty() ? std::to_string(value) : accum + "," + std::to_string(value);
  };

  /* Add eeg_data if available. */
  if (!msg->eeg_data.empty()) {
      preprocessed_buffer << ",";
      std::string eeg_str = std::accumulate(std::begin(msg->eeg_data), std::end(msg->eeg_data), std::string{}, comma_join);
      preprocessed_buffer << eeg_str;
  }

  /* Add emg_data if available. */
  if (!msg->emg_data.empty()) {
      preprocessed_buffer << ",";
      std::string emg_str = std::accumulate(std::begin(msg->emg_data), std::end(msg->emg_data), std::string{}, comma_join);
      preprocessed_buffer << emg_str;
  }

  preprocessed_buffer << "\n";

  /* Update sample count. */
  preprocessed_sample_count++;

  if (preprocessed_sample_count % BATCH_SIZE_IN_SAMPLES == 0) {
    write_preprocessed_buffer();
  }
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<EegRecorder>();

  rclcpp::executors::StaticSingleThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
