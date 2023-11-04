#include <chrono>
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

using namespace std::placeholders;

const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_TRIGGER_TOPIC = "/eeg/trigger";
const std::string DATASET_LIST_TOPIC = "/eeg_simulator/dataset/list";

const std::string DATA_DIRECTORY = "data/eeg/";
const std::string PROJECTS_DIRECTORY = "projects/";


/* TODO: Simulating the EEG device to the level of sending UDP packets not implemented on the C++
     side yet. For a previous Python reference implementation, see commit c0afb515b. */
EegSimulator::EegSimulator() : Node("eeg_simulator") {
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

  /* Publisher for EEG samples. */
  eeg_publisher = this->create_publisher<eeg_interfaces::msg::EegSample>(EEG_RAW_TOPIC, 10);
  trigger_publisher = this->create_publisher<eeg_interfaces::msg::Trigger>(EEG_TRIGGER_TOPIC, 10);
  eeg_info_publisher = this->create_publisher<eeg_interfaces::msg::EegInfo>(EEG_INFO_TOPIC, qos_persist_latest);

  /* TODO: Comment out for now. */
  //publish_sample_timer_ = this->create_wall_timer(std::chrono::duration<double>(sampling_period), std::bind(&EegSimulator::publish_sample, this));
  //publish_trigger_timer_ = this->create_wall_timer(std::chrono::seconds(5), std::bind(&EegSimulator::publish_trigger, this));

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
      RCLCPP_INFO(this->get_logger(), "Found the dataset: %s.", filename.c_str());

      try {
        project_interfaces::msg::Dataset dataset_msg;
        file >> json_data;

        dataset_msg.filename = filename;
        dataset_msg.name = json_data.value("name", "Unknown");

        if (json_data.contains("channels") && json_data["channels"].is_object()) {
            dataset_msg.num_of_eeg_channels = json_data["channels"].value("eeg", 0);
            dataset_msg.num_of_emg_channels = json_data["channels"].value("emg", 0);
        } else {
            dataset_msg.num_of_eeg_channels = 0;
            dataset_msg.num_of_emg_channels = 0;
        }

        dataset_msg.sampling_frequency = json_data.value("sampling_frequency", 0);
        dataset_msg.duration = json_data.value("duration", 0);

        datasets.push_back(dataset_msg);

      } catch (const nlohmann::json::parse_error& ex) {
        RCLCPP_ERROR(this->get_logger(), "JSON parse error with dataset %s: %s", filename.c_str(), ex.what());
      }
    }
  }
  return datasets;
}

void EegSimulator::update_dataset_list() {
  auto datasets = this->list_datasets(this->dataset_directory);

  /* Publish datasets. */
  project_interfaces::msg::DatasetList msg;
  msg.datasets = datasets;

  this->dataset_list_publisher->publish(msg);

  /* Store datasets internally. */
  for (const auto& dataset : datasets) {
    dataset_map[dataset.filename] = dataset;
  }
}

void EegSimulator::handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg) {
  this->active_project = msg->data;

  std::ostringstream oss;
  oss << PROJECTS_DIRECTORY << this->active_project << "/eeg/raw";
  this->dataset_directory = oss.str();

  RCLCPP_INFO(this->get_logger(), "Active project set to: %s.", this->active_project.c_str());

  update_dataset_list();

  update_inotify_watch();
}

void EegSimulator::handle_set_dataset(
      const std::shared_ptr<project_interfaces::srv::SetDataset::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDataset::Response> response) {

  if (dataset_map.find(request->filename) == dataset_map.end()) {
    RCLCPP_ERROR(this->get_logger(), "Dataset not found: %s.", request->filename.c_str());

    response->success = false;
    return;
  }
  this->dataset_filename = request->filename;

  RCLCPP_INFO(this->get_logger(), "Dataset set to: %s.", this->dataset_filename.c_str());

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::String();
  msg.data = this->dataset_filename;

  this->dataset_publisher->publish(msg);

  /* Update dataset internally. */
  this->dataset = dataset_map[dataset_filename];

  /* If playback is set to true, re-initialize streaming. */
  if (this->playback) {
    initialize_streaming();
  }

  response->success = true;
}

void EegSimulator::handle_set_playback(
      const std::shared_ptr<project_interfaces::srv::SetPlayback::Request> request,
      std::shared_ptr<project_interfaces::srv::SetPlayback::Response> response) {

  this->playback= request->playback;

  RCLCPP_INFO(this->get_logger(), "Playback %s.", this->playback ? "enabled" : "disabled");

  /* Update ROS state variable. */
  auto msg = std_msgs::msg::Bool();
  msg.data = this->playback;

  this->playback_publisher->publish(msg);

  /* If playback is set to true, initialize streaming. */
  if (this->playback) {
    initialize_streaming();
  }

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
  sampling_frequency = this->dataset.sampling_frequency;
  num_of_eeg_channels = this->dataset.num_of_eeg_channels;
  num_of_emg_channels = this->dataset.num_of_emg_channels;
  csv_filename = this->dataset.filename + ".csv";

  std::string dataset_path = dataset_directory + "/" + csv_filename;

  this->sampling_period = 1.0 / this->dataset.sampling_frequency;
  this->total_channels = this->dataset.num_of_eeg_channels + this->dataset.num_of_emg_channels;

  file.open(dataset_path, std::ios::in);

  RCLCPP_INFO(this->get_logger(), "Reading data from file: %s", dataset_path.c_str());

  /* Publish EEG info. */
  eeg_interfaces::msg::EegInfo eeg_info;

  eeg_info.sampling_frequency = sampling_frequency;
  eeg_info.num_of_eeg_channels = num_of_eeg_channels;
  eeg_info.num_of_emg_channels = num_of_emg_channels;
  eeg_info.send_trigger_as_channel = false;

  eeg_info_publisher->publish(eeg_info);
}

void EegSimulator::publish_sample() {
  std::string line;
  std::getline(file, line);

  if (line.empty() && loop) {
    file.clear();
    file.seekg(0, std::ios::beg);
    std::getline(file, line);
  } else if (line.empty() && !loop) {
    RCLCPP_INFO(this->get_logger(), "Published all samples from file");
    return;
  }

  std::stringstream ss(line);
  std::string number;
  std::vector<double> data;

  while (std::getline(ss, number, ',')) {
    data.push_back(std::stod(number));
  }

  if (total_channels > static_cast<int>(data.size())) {
    RCLCPP_ERROR(this->get_logger(), "Total # of EEG and EMG channels (%d) exceeds # of channels in data (%zu)", total_channels, data.size());
    return;
  }

  eeg_interfaces::msg::EegSample msg;

  msg.eeg_data.insert(msg.eeg_data.end(), data.begin(), data.begin() + num_of_eeg_channels);
  msg.emg_data.insert(msg.emg_data.end(), data.begin() + num_of_eeg_channels, data.begin() + total_channels);
  msg.first_sample_of_session = current_time > 0 ? false : true;
  msg.time = current_time;

  eeg_publisher->publish(msg);
  if (static_cast<int>(current_time) > static_cast<int>(last_log_time)) {
    RCLCPP_INFO(this->get_logger(), "Published EEG datapoint in topic %s with timestamp %.4f s.", EEG_RAW_TOPIC.c_str(), current_time);
    last_log_time = current_time;
  }

  current_time += sampling_period;
}

void EegSimulator::publish_trigger() {
  RCLCPP_INFO(this->get_logger(), "Publishing trigger");

  eeg_interfaces::msg::Trigger msg;
  msg.time = current_time;

  trigger_publisher->publish(msg);
}

/* Inotify functions */

void EegSimulator::update_inotify_watch() {
  /* Remove the old watch. */
  inotify_rm_watch(inotify_descriptor, watch_descriptor);

  /* Add a new watch. */
  watch_descriptor = inotify_add_watch(inotify_descriptor, this->dataset_directory.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
  if (watch_descriptor == -1) {
      RCLCPP_ERROR(this->get_logger(), "Error adding watch for: %s", this->dataset_directory.c_str());
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
