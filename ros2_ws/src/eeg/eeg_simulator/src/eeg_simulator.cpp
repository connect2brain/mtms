#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <thread>
#include <string>

#include "eeg_simulator.h"


const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_TRIGGER_TOPIC = "/eeg/trigger";
const std::string DATA_DIRECTORY = "data/eeg/";
const std::string DATASET_LIST_TOPIC = "/eeg_simulator/dataset/list";

const std::string PROJECTS_DIRECTORY = "projects/";

/* TODO: Simulating the EEG device to the level of sending UDP packets not implemented on the C++
     side yet. For a previous Python reference implementation, see commit c0afb515b. */
EegSimulator::EegSimulator() : Node("eeg_simulator") {
  /* Publisher for EEG samples. */
  eeg_publisher_ = this->create_publisher<eeg_interfaces::msg::EegSample>(EEG_RAW_TOPIC, 10);
  trigger_publisher_ = this->create_publisher<eeg_interfaces::msg::Trigger>(EEG_TRIGGER_TOPIC, 10);
  eeg_info_publisher_ = this->create_publisher<eeg_interfaces::msg::EegInfo>(EEG_INFO_TOPIC, rclcpp::QoS(1).transient_local());

  this->declare_parameter<std::string>("data_file", "");
  data_file_name = this->get_parameter("data_file").as_string();

  this->declare_parameter<int>("sampling_frequency", 0);
  sampling_frequency = this->get_parameter("sampling_frequency").as_int();

  sampling_period = 1.0 / sampling_frequency;

  this->declare_parameter<bool>("loop", false);
  loop = this->get_parameter("loop").as_bool();

  this->declare_parameter<int>("num_of_eeg_channels", 0);
  num_of_eeg_channels = this->get_parameter("num_of_eeg_channels").as_int();

  this->declare_parameter<int>("num_of_emg_channels", 0);
  num_of_emg_channels = this->get_parameter("num_of_emg_channels").as_int();
  total_channels = num_of_eeg_channels + num_of_emg_channels;

  RCLCPP_INFO(this->get_logger(), "Reading data from file %s in directory %s.", data_file_name.c_str(), DATA_DIRECTORY.c_str());

  data_path = DATA_DIRECTORY + data_file_name;

  /* HACK: When EEG simulator is started simultaneously with EEG processor, it may start streaming already
        before EEG processor is up and running. Hence, wait for several seconds here to ensure that EEG
        processor is running. The correct fix would be to make starting and stopping the EEG simulator more
        explicit, e.g., so that it is done using start-session ROS service, as it is done with the real
        EEG device. */
  std::this_thread::sleep_for(std::chrono::seconds(3));

  file.open(data_path, std::ios::in);
  publish_sample_timer_ = this->create_wall_timer(std::chrono::duration<double>(sampling_period), std::bind(&EegSimulator::publish_sample, this));
  publish_trigger_timer_ = this->create_wall_timer(std::chrono::seconds(5), std::bind(&EegSimulator::publish_trigger, this));

  eeg_interfaces::msg::EegInfo eeg_info;

  eeg_info.sampling_frequency = sampling_frequency;
  eeg_info.num_of_eeg_channels = num_of_eeg_channels;
  eeg_info.num_of_emg_channels = num_of_emg_channels;
  eeg_info.send_trigger_as_channel = false;

  eeg_info_publisher_->publish(eeg_info);
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

  eeg_publisher_->publish(msg);
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

  trigger_publisher_->publish(msg);
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
