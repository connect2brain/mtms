#include <chrono>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <thread>
#include <string>
#include <filesystem>

#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <nlohmann/json.hpp>

#include "eeg_simulator.h"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::placeholders;

const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_TRIGGER_TOPIC = "/eeg/trigger";
const std::string DATASET_LIST_TOPIC = "/eeg_simulator/dataset/list";

const std::string PROJECTS_DIRECTORY = "projects/";
const std::string EEG_SIMULATOR_DATA_SUBDIRECTORY = "eeg_simulator/";

const milliseconds SESSION_PUBLISHING_INTERVAL = 1ms;
const milliseconds SESSION_PUBLISHING_INTERVAL_TOLERANCE = 2ms;

/* Have a long queue to avoid dropping messages. */
const uint16_t EEG_QUEUE_LENGTH = 65535;


/* TODO: Simulating the EEG device to the level of sending UDP packets not implemented on the C++
     side yet. For a previous Python reference implementation, see commit c0afb515b. */
EegSimulator::EegSimulator() : Node("eeg_simulator") {

  /* Subscriber for EEG bridge healthcheck. */
  this->eeg_bridge_healthcheck_subscriber = create_subscription<system_interfaces::msg::Healthcheck>(
    "/eeg/healthcheck",
    10,
    std::bind(&EegSimulator::handle_eeg_bridge_healthcheck, this, _1));

  /* Publisher for EEG simulator healthcheck. */
  this->healthcheck_publisher = this->create_publisher<system_interfaces::msg::Healthcheck>(
    "/eeg_simulator/healthcheck",
    10);

  /* Subscriber for active project. */
  auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

  this->active_project_subscriber = create_subscription<std_msgs::msg::String>(
    "/projects/active",
    qos_persist_latest,
    std::bind(&EegSimulator::handle_set_active_project, this, _1));

  /* Publisher for EEG datasets. */
  dataset_list_publisher = this->create_publisher<project_interfaces::msg::DatasetList>(
    DATASET_LIST_TOPIC,
    qos_persist_latest);

  /* Service for changing dataset. */
  this->set_dataset_service = this->create_service<project_interfaces::srv::SetDataset>(
    "/eeg_simulator/dataset/set",
    std::bind(&EegSimulator::handle_set_dataset, this, _1, _2));

  /* Service for changing playback. */
  this->set_playback_service = this->create_service<project_interfaces::srv::SetPlayback>(
    "/eeg_simulator/playback/set",
    std::bind(&EegSimulator::handle_set_playback, this, _1, _2));

  /* Service for changing loop. */
  this->set_loop_service = this->create_service<project_interfaces::srv::SetLoop>(
    "/eeg_simulator/loop/set",
    std::bind(&EegSimulator::handle_set_loop, this, _1, _2));

  /* Publisher for dataset. */
  this->dataset_publisher = this->create_publisher<std_msgs::msg::String>(
    "/eeg_simulator/dataset",
    qos_persist_latest);

  /* Publisher for playback. */
  this->playback_publisher = this->create_publisher<std_msgs::msg::Bool>(
    "/eeg_simulator/playback",
    qos_persist_latest);

  /* Publisher for loop. */
  this->loop_publisher = this->create_publisher<std_msgs::msg::Bool>(
    "/eeg_simulator/loop",
    qos_persist_latest);

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
    std::bind(&EegSimulator::handle_session, this, _1));

  /* Publisher for EEG samples. */
  eeg_publisher = this->create_publisher<eeg_interfaces::msg::Sample>(
    EEG_RAW_TOPIC,
    EEG_QUEUE_LENGTH);

  trigger_publisher = this->create_publisher<eeg_interfaces::msg::Trigger>(EEG_TRIGGER_TOPIC, 10);
  eeg_info_publisher = this->create_publisher<eeg_interfaces::msg::EegInfo>(EEG_INFO_TOPIC, qos_persist_latest);

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
  this->inotify_timer = this->create_wall_timer(std::chrono::milliseconds(100),
                                                std::bind(&EegSimulator::inotify_timer_callback, this));
}

EegSimulator::~EegSimulator() {
  inotify_rm_watch(inotify_descriptor, watch_descriptor);
  close(inotify_descriptor);
}

void EegSimulator::publish_healthcheck(uint8_t status, std::string status_message, std::string actionable_message) {
  auto healthcheck = system_interfaces::msg::Healthcheck();

  healthcheck.status.value = status;
  healthcheck.status_message = status_message;
  healthcheck.actionable_message = actionable_message;

  this->healthcheck_publisher->publish(healthcheck);
}

void EegSimulator::handle_eeg_bridge_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) {
  bool eeg_bridge_available = msg->status.value == system_interfaces::msg::HealthcheckStatus::READY;

  if (eeg_bridge_available) {
    this->publish_healthcheck(system_interfaces::msg::HealthcheckStatus::NOT_READY,
                             "EEG bridge is available, which is mutually exclusive with the EEG simulator.",
                             "Turn off the EEG bridge to use the EEG simulator.");
    this->set_playback(false);

    RCLCPP_INFO(this->get_logger(), "EEG simulator disabled because EEG bridge is available.");
  } else {
    this->publish_healthcheck(system_interfaces::msg::HealthcheckStatus::READY,
                             "Ready",
                             "");
  }
}

std::tuple<int, double, bool> EegSimulator::get_dataset_info(const std::string& data_file_path) {
  std::ifstream data_file(data_file_path);
  int sampling_frequency = 0;
  double duration = 0.0;
  bool samples_dropped = false;

  if (data_file.is_open()) {
    std::string line;
    double first_timestamp, second_timestamp;

    /* Read the first timestamp. */
    if (std::getline(data_file, line)) {
      std::stringstream ss(line);
      ss >> first_timestamp;
    }

    /* Read the second timestamp to determine sampling frequency. */
    if (std::getline(data_file, line)) {
      std::stringstream ss(line);
      ss >> second_timestamp;

      /* Calculate and round the sampling frequency to the nearest integer. */
      sampling_frequency = static_cast<int>(std::round(1.0 / (second_timestamp - first_timestamp)));
    }

    double_t previous_timestamp = second_timestamp;
    double_t expected_difference = second_timestamp - first_timestamp;

    double_t internal_timestamp;

    /* Read until the last line to get the last timestamp and check for dropped samples. */
    while (std::getline(data_file, line)) {
      std::stringstream ss(line);
      ss >> internal_timestamp;

      /* Check if the current sample interval is equal to the expected interval (second - first). */
      if (std::abs((internal_timestamp - previous_timestamp) - expected_difference) > TOLERANCE_S) {
        RCLCPP_WARN(this->get_logger(), "Warning: Dropped samples found in dataset %s.", data_file_path.c_str());
        RCLCPP_WARN(this->get_logger(), "Previous timestamp: %.4f, current timestamp: %.4f, expected difference: %.4f",
          previous_timestamp,
          internal_timestamp,
          expected_difference);

        samples_dropped = true;
      }
      previous_timestamp = internal_timestamp;
    }

    /* Calculate the duration using the first and last timestamp. */
    duration = internal_timestamp - first_timestamp;
  } else {
    throw std::runtime_error("Failed to open data file: " + data_file_path);
  }

  return std::make_tuple(sampling_frequency, duration, samples_dropped);
}

std::vector<project_interfaces::msg::Dataset> EegSimulator::list_datasets(const std::string& path) {
  std::vector<project_interfaces::msg::Dataset> datasets;

  /* Check that the directory exists. */
  if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
    RCLCPP_WARN(this->get_logger(), "Warning: Directory does not exist: %s.", path.c_str());
    return datasets;
  }

  /* List all .json files in the directory and fetch their attributes. */
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".json") {
      std::ifstream file(entry.path());
      nlohmann::json json_data;

      std::string filename = entry.path().filename().string();
      RCLCPP_INFO(this->get_logger(), "Found the dataset: %s", filename.c_str());

      try {
        project_interfaces::msg::Dataset dataset_msg;
        file >> json_data;

        dataset_msg.json_filename = filename;
        dataset_msg.name = json_data.value("name", "Unknown");
        dataset_msg.data_filename = json_data.value("data_file", "");
        dataset_msg.trigger_filename = json_data.value("trigger_file", "");

        if (json_data.contains("channels") && json_data["channels"].is_object()) {
            dataset_msg.num_of_eeg_channels = json_data["channels"].value("eeg", 0);
            dataset_msg.num_of_emg_channels = json_data["channels"].value("emg", 0);
        } else {
            dataset_msg.num_of_eeg_channels = 0;
            dataset_msg.num_of_emg_channels = 0;
        }

        /* Get the sampling frequency and duration from the data file. */
        std::string data_file_path = entry.path().parent_path().string() + "/" + dataset_msg.data_filename;
        auto [sampling_frequency, duration, samples_dropped] = get_dataset_info(data_file_path);

        if (samples_dropped) {
          /* TODO: Should this fail harder? */
          RCLCPP_WARN(this->get_logger(), "Warning: Dropped samples found in dataset %s.", filename.c_str());
        }

        dataset_msg.sampling_frequency = sampling_frequency;
        dataset_msg.duration = duration;

        datasets.push_back(dataset_msg);

      } catch (const nlohmann::json::parse_error& ex) {
        RCLCPP_ERROR(this->get_logger(), "JSON parse error with dataset %s: %s", filename.c_str(), ex.what());
      }
    }
  }
  return datasets;
}

void EegSimulator::update_dataset_list() {
  auto datasets = this->list_datasets(this->data_directory);

  /* Publish datasets. */
  project_interfaces::msg::DatasetList msg;
  msg.datasets = datasets;

  this->dataset_list_publisher->publish(msg);

  /* Store datasets internally. */
  for (const auto& dataset : datasets) {
    dataset_map[dataset.json_filename] = dataset;
  }

  /* XXX: Maybe not the cleanest way to pass on the default dataset to the caller or whoever needs it. */
  this->default_dataset_json = datasets.size() > 0 ? datasets[0].json_filename : "";
}

void EegSimulator::handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  std::ostringstream oss;
  oss << PROJECTS_DIRECTORY << "/" << this->active_project << "/" << EEG_SIMULATOR_DATA_SUBDIRECTORY;
  this->data_directory = oss.str();

  RCLCPP_INFO(this->get_logger(), "Active project set to: %s", this->active_project.c_str());

  update_dataset_list();

  /* When the project changes, first set the default dataset. */
  [[maybe_unused]] bool success = set_dataset(this->default_dataset_json);

  update_inotify_watch();
}

bool EegSimulator::set_dataset(std::string json_filename) {
  if (dataset_map.find(json_filename) == dataset_map.end()) {
    RCLCPP_ERROR(this->get_logger(), "Dataset not found: %s.", json_filename.c_str());

    return false;
  }
  /* Update ROS state variable. */
  auto msg = std_msgs::msg::String();
  msg.data = json_filename;

  this->dataset_publisher->publish(msg);

  /* Update dataset internally. */
  this->dataset = dataset_map[json_filename];

  /* If playback is set to true, re-initialize streaming. */
  if (this->playback) {
    initialize_streaming();
  }

  RCLCPP_INFO(this->get_logger(), "Dataset JSON set to: %s", json_filename.c_str());

  return true;
}

void EegSimulator::handle_set_dataset(
      const std::shared_ptr<project_interfaces::srv::SetDataset::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDataset::Response> response) {

  bool success = set_dataset(request->filename);

  response->success = success;
}

void EegSimulator::set_playback(bool playback) {
  this->playback = playback;

  RCLCPP_INFO(this->get_logger(), "Playback %s.", this->playback ? "enabled" : "disabled");

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::Bool();
  msg.data = this->playback;

  this->playback_publisher->publish(msg);

  /* If playback is set to true, initialize streaming. */
  if (this->playback) {
    initialize_streaming();
  }
}

void EegSimulator::handle_set_playback(
      const std::shared_ptr<project_interfaces::srv::SetPlayback::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPlayback::Response> response) {

  set_playback(request->playback);
  response->success = true;
}

void EegSimulator::handle_set_loop(
      const std::shared_ptr<project_interfaces::srv::SetLoop::Request> request,
      std::shared_ptr<project_interfaces::srv::SetLoop::Response> response) {

  this->loop= request->loop;

  RCLCPP_INFO(this->get_logger(), "Loop %s.", this->loop ? "enabled" : "disabled");

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::Bool();
  msg.data = this->loop;

  this->loop_publisher->publish(msg);

  response->success = true;
}

void EegSimulator::initialize_streaming() {
  this->sampling_frequency = this->dataset.sampling_frequency;
  this->num_of_eeg_channels = this->dataset.num_of_eeg_channels;
  this->num_of_emg_channels = this->dataset.num_of_emg_channels;

  /* Initialize variables. */
  this->total_channels = this->num_of_eeg_channels + this->num_of_emg_channels;
  this->sampling_period = 1.0 / this->sampling_frequency;

  this->dataset_time = 0.0;
  this->time_offset = latest_session_time;

  /* Open data file. */
  std::string data_filename = this->dataset.data_filename;
  std::string data_file_path = data_directory + "/" + data_filename;

  if (data_file.is_open()) {
      data_file.close();
  }
  data_file.open(data_file_path, std::ios::in);

  if (!data_file.is_open()) {
    RCLCPP_ERROR(this->get_logger(), "Error opening file: %s", data_file_path.c_str());
    return;
  }

  RCLCPP_INFO(this->get_logger(), "Reading data from file: %s", data_file_path.c_str());

  /* Open trigger file. */
  std::string trigger_filename = this->dataset.trigger_filename;
  std::string trigger_file_path = data_directory + "/" + trigger_filename;

  this->send_triggers = false;
  if (trigger_filename != UNSET_FILENAME) {
    if (trigger_file.is_open()) {
        trigger_file.close();
    }
    trigger_file.open(trigger_file_path, std::ios::in);

    if (!trigger_file.is_open()) {
      RCLCPP_ERROR(this->get_logger(), "Error opening file: %s", trigger_file_path.c_str());
    }
    RCLCPP_INFO(this->get_logger(), "Reading triggers from file: %s", trigger_file_path.c_str());

    /* Read the next trigger time from the file already here so the session handler loop doesn't have to worry about it. */
    read_next_trigger_time();

    this->send_triggers = true;
  } else {
    RCLCPP_INFO(this->get_logger(), "No trigger file defined.");
  }

  /* Publish EEG info. */
  eeg_interfaces::msg::EegInfo eeg_info;

  eeg_info.sampling_frequency = this->sampling_frequency;
  eeg_info.num_of_eeg_channels = this->num_of_eeg_channels;
  eeg_info.num_of_emg_channels = this->num_of_emg_channels;
  eeg_info.send_trigger_as_channel = false;

  eeg_info_publisher->publish(eeg_info);
}

void EegSimulator::handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg) {
  auto current_time = msg->time;

  this->latest_session_time = current_time;

  /* Return if session is not ongoing. */
  if (msg->state.value != system_interfaces::msg::SessionState::STARTED) {
    /* Re-initialize streaming already after stopping the previous session. */
    if (this->session_started) {
      RCLCPP_INFO(this->get_logger(), "Session stopped, stopping streaming.");
      this->initialize_streaming();
    }

    this->session_started = false;
    return;
  }

  /* If this is the first time the session is started, set the time offset. */
  if (!this->session_started) {
    RCLCPP_INFO(this->get_logger(), "Session started, starting streaming.");

    this->session_started = true;
    this->first_sample_of_session = true;

    time_offset = current_time;
  }

  /* Return if the simulator is not set to playback dataset. */
  if (!playback) {
    return;
  }

  bool stop = false;
  bool looped;
  double_t sample_time;

  /* Loop until the dataset time adjusted by the offset reaches the current time OR
     streaming needs to stop (e.g., the dataset is finished or there is an error). */
  while (dataset_time + time_offset <= current_time && !stop) {
    std::tie(stop, looped, sample_time) = publish_sample(current_time);

    if (looped) {
      /* If dataset looped, reset the trigger file. */
      if (this->send_triggers) {
        trigger_file.clear();
        trigger_file.seekg(0, std::ios::beg);

        read_next_trigger_time();
      }
    }
    if (this->send_triggers) {
      publish_triggers_up_to(sample_time);
    }

    /* TODO: The problem with current design is that it publishes the next sample too early; the dataset time is updated only
         after the sample is published. Doing this properly would require a "look-ahead" mechanism, where the time to publish
         next sample would be peeked in advance.

         On the other hand, because triggers and samples are sent asynchronously, being a bit ahead in time ensures that both
         the trigger and the corresponding sample have arrived to the subscriber before the session reaches that time. */
    dataset_time = sample_time;
  }

  /* If dataset is finished OR there is an error, automatically disable playback. */
  if (stop) {
    this->set_playback(false);
  }
}

std::tuple<bool, bool, double_t> EegSimulator::publish_sample(double_t current_time) {
  bool looped = false;
  std::string line;

  if (!std::getline(data_file, line)) {
    if (data_file.eof()) {
      if (loop) {
        RCLCPP_INFO(this->get_logger(), "Reached the end of file, looping back to beginning.");

        data_file.clear();
        data_file.seekg(0, std::ios::beg);
        std::getline(data_file, line);

        /* If the dataset looped, update the time offset. */
        time_offset = current_time;

        looped = true;
      } else {
        RCLCPP_INFO(this->get_logger(), "Reached the end of file.");

        return {true, looped, 0.0};
      }
    }
  }

  std::stringstream ss(line);
  std::string number;
  std::vector<double_t> data;
  double_t sample_time;

  /* First read the time (the first column). */
  if (std::getline(ss, number, ',')) {
    sample_time = std::stod(number);
  } else {
    RCLCPP_ERROR(this->get_logger(), "Failed to read time column from data file.");
    return {true, looped, 0.0};
  }

  /* Then read the EEG and EMG data (the next columns). */
  while (std::getline(ss, number, ',')) {
      data.push_back(std::stod(number));
  }

  if (total_channels > static_cast<int>(data.size())) {
    RCLCPP_ERROR(this->get_logger(), "Total # of EEG and EMG channels (%d) exceeds # of channels in data (%zu)", total_channels, data.size());
    return {true, looped, 0.0};
  }

  double_t time = sample_time + time_offset;

  eeg_interfaces::msg::Sample msg;

  msg.eeg_data.insert(msg.eeg_data.end(), data.begin(), data.begin() + num_of_eeg_channels);
  msg.emg_data.insert(msg.emg_data.end(), data.begin() + num_of_eeg_channels, data.begin() + total_channels);

  msg.metadata.first_sample_of_session = this->first_sample_of_session;
  msg.metadata.sampling_frequency = this->sampling_frequency;
  msg.metadata.num_of_eeg_channels = this->num_of_eeg_channels;
  msg.metadata.num_of_emg_channels = this->num_of_emg_channels;

  msg.time = time;

  eeg_publisher->publish(msg);

  /* Update 'first sample of session'. */
  this->first_sample_of_session = false;

  RCLCPP_INFO_THROTTLE(this->get_logger(),
                       *this->get_clock(),
                       1000,
                       "Published EEG datapoint in topic %s with timestamp %.4f s.",
                       EEG_RAW_TOPIC.c_str(),
                       time);

  return {false, looped, sample_time};
}

void EegSimulator::read_next_trigger_time() {
  triggers_left = false;
  std::string line;

  if (!std::getline(trigger_file, line)) {
    if (trigger_file.eof()) {
      RCLCPP_INFO(this->get_logger(), "Reached the end of trigger file.");
      return;
    }
  }

  try {
    next_trigger_time = std::stod(line);
    triggers_left = true;

  } catch (const std::invalid_argument& e) {
    RCLCPP_ERROR(this->get_logger(), "Invalid number in trigger file.");

  } catch (const std::out_of_range& e) {
    RCLCPP_ERROR(this->get_logger(), "Number out of range in trigger file.");
  }
}

void EegSimulator::publish_triggers_up_to(double_t time) {
  while (triggers_left && next_trigger_time < time) {
    double_t trigger_time = next_trigger_time + time_offset;

    /* Publish the trigger. */
    eeg_interfaces::msg::Trigger msg;
    msg.time = trigger_time;

    trigger_publisher->publish(msg);

    RCLCPP_INFO(this->get_logger(), "Published trigger in topic %s with timestamp %.4f s.", EEG_TRIGGER_TOPIC.c_str(), trigger_time);

    /* Read the next trigger. */
    read_next_trigger_time();
  }
}

/* Inotify functions */

void EegSimulator::update_inotify_watch() {
  /* Remove the old watch. */
  inotify_rm_watch(inotify_descriptor, watch_descriptor);

  /* Add a new watch. */
  watch_descriptor = inotify_add_watch(inotify_descriptor, this->data_directory.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
  if (watch_descriptor == -1) {
      RCLCPP_ERROR(this->get_logger(), "Error adding watch for: %s", this->data_directory.c_str());
      return;
  }
}

void EegSimulator::inotify_timer_callback() {
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
      if (event->mask & (IN_CREATE | IN_DELETE)) {
        RCLCPP_INFO(this->get_logger(), "File '%s' created or deleted, updating dataset list.", event_name.c_str());
        this->update_dataset_list();
      }
    }
    i += sizeof(struct inotify_event) + event->len;
  }
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_simulator"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegSimulator>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_simulator"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
