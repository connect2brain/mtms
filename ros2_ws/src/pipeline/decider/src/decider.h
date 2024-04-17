//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_DECIDER_H
#define EEG_PROCESSOR_DECIDER_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"

#include "eeg_interfaces/msg/sample.hpp"
#include "eeg_interfaces/msg/preprocessed_sample.hpp"
#include "eeg_interfaces/msg/trigger.hpp"

#include "event_interfaces/msg/event_trigger.hpp"
#include "event_interfaces/msg/ready_for_event_trigger.hpp"

#include "experiment_interfaces/msg/trial_feedback.hpp"

#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

#include "system_interfaces/msg/session.hpp"
#include "system_interfaces/msg/session_state.hpp"

#include "pipeline_interfaces/msg/latency.hpp"
#include "pipeline_interfaces/msg/sensory_stimulus.hpp"

#include "project_interfaces/msg/decider_list.hpp"
#include "project_interfaces/srv/set_decider_module.hpp"
#include "project_interfaces/srv/set_decider_enabled.hpp"

#include "ring_buffer.h"

const uint16_t UNSET_SAMPLING_FREQUENCY = 0;
const uint8_t UNSET_NUM_OF_CHANNELS = 255;
const double_t UNSET_PREVIOUS_TIME = std::numeric_limits<double_t>::quiet_NaN();
const std::string UNSET_STRING = "";

enum class DeciderState {
  WAITING_FOR_ENABLED,
  READY,
  SAMPLES_DROPPED,
  MODULE_ERROR
};

class DeciderWrapper;

class EegDecider : public rclcpp::Node {
public:
  EegDecider();
  ~EegDecider();

private:
  void publish_healthcheck();

  void handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg);

  void handle_session(const std::shared_ptr<system_interfaces::msg::Session> msg);

  void update_eeg_info(const eeg_interfaces::msg::PreprocessedSampleMetadata& msg);
  void initialize_module();

  void reset_decider_state();

  void unset_decider_module();

  bool set_decider_enabled(bool enabled);
  void handle_set_decider_enabled(
      const std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderEnabled::Response> response);

  bool set_decider_module(const std::string module);
  void handle_set_decider_module(
      const std::shared_ptr<project_interfaces::srv::SetDeciderModule::Request> request,
      std::shared_ptr<project_interfaces::srv::SetDeciderModule::Response> response);

  void handle_set_active_project(const std::shared_ptr<std_msgs::msg::String> msg);
  std::vector<std::string> list_python_modules(const std::string& path);
  void update_decider_list();

  void check_dropped_samples(double_t sample_time);

  void calculate_latency(double_t pulse_execution_time);
  void handle_eeg_trigger(const std::shared_ptr<eeg_interfaces::msg::Trigger> msg);
  void handle_trial_feedback(const std::shared_ptr<experiment_interfaces::msg::TrialFeedback> msg);

  void update_ready_for_trigger(const std::shared_ptr<event_interfaces::msg::ReadyForEventTrigger> msg);

  void process_sample(const std::shared_ptr<eeg_interfaces::msg::PreprocessedSample> msg);

  /* Inotify functions */
  void update_inotify_watch();
  void inotify_timer_callback();

  rclcpp::Logger logger;

  rclcpp::Subscription<system_interfaces::msg::Healthcheck>::SharedPtr mtms_device_healthcheck_subscriber;

  rclcpp::TimerBase::SharedPtr healthcheck_publisher_timer;
  rclcpp::Publisher<system_interfaces::msg::Healthcheck>::SharedPtr healthcheck_publisher;

  rclcpp::Subscription<system_interfaces::msg::Session>::SharedPtr session_subscriber;

  rclcpp::Subscription<eeg_interfaces::msg::PreprocessedSample>::SharedPtr preprocessed_eeg_subscriber;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr active_project_subscriber;
  rclcpp::Publisher<project_interfaces::msg::DeciderList>::SharedPtr decider_list_publisher;

  rclcpp::Service<project_interfaces::srv::SetDeciderModule>::SharedPtr set_decider_module_service;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr decider_module_publisher;

  rclcpp::Service<project_interfaces::srv::SetDeciderEnabled>::SharedPtr set_decider_enabled_service;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr decider_enabled_publisher;

  rclcpp::Publisher<event_interfaces::msg::EventTrigger>::SharedPtr trigger_publisher;
  rclcpp::Publisher<event_interfaces::msg::EventTrigger>::SharedPtr external_trigger_publisher;
  rclcpp::Subscription<event_interfaces::msg::ReadyForEventTrigger>::SharedPtr ready_for_trigger_subscriber;

  rclcpp::Subscription<experiment_interfaces::msg::TrialFeedback>::SharedPtr trial_feedback_subscriber;
  rclcpp::Subscription<eeg_interfaces::msg::Trigger>::SharedPtr eeg_trigger_subscriber;

  rclcpp::Publisher<pipeline_interfaces::msg::Latency>::SharedPtr latency_publisher;
  rclcpp::Publisher<pipeline_interfaces::msg::SensoryStimulus>::SharedPtr sensory_stimulus_publisher;

  bool enabled = false;

  DeciderState decider_state = DeciderState::WAITING_FOR_ENABLED;
  bool first_sample = true;

  /* Used for keeping track of the time of the previous trigger time to ensure that the minimum pulse
     interval is respected. */
  double_t previous_trigger_time = UNSET_PREVIOUS_TIME;

  bool reinitialize = false;

  std::string active_project = UNSET_STRING;

  std::string script_directory  = UNSET_STRING;
  std::string module_name = UNSET_STRING;

  std::vector<std::string> modules;

  /* Information about the EEG device configuration. */
  uint16_t sampling_frequency = UNSET_SAMPLING_FREQUENCY;
  uint8_t num_of_eeg_channels = UNSET_NUM_OF_CHANNELS;
  uint8_t num_of_emg_channels = UNSET_NUM_OF_CHANNELS;
  double_t sampling_period;

  /* For checking if samples have been dropped, store the time of the previous sample received. */
  double_t previous_time = UNSET_PREVIOUS_TIME;

  /* For latency calculation, store the times of the previous pulse decisions in a queue. */
  std::queue<double_t> decision_times;

  /* 'Ready for trigger' is set to true when the mTMS is ready to trigger the next pulse. */
  bool ready_for_trigger = false;

  RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedSample>> sample_buffer;
  pipeline_interfaces::msg::SensoryStimulus sensory_stimulus;

  std::unique_ptr<DeciderWrapper> decider_wrapper;

  /* ROS parameters */
  double_t minimum_pulse_interval;

  /* Healthcheck */
  uint8_t status;
  std::string status_message;
  std::string actionable_message;

  /* mTMS device healthcheck */
  bool mtms_device_available = false;

  /* Inotify variables */
  rclcpp::TimerBase::SharedPtr inotify_timer;
  int inotify_descriptor;
  int watch_descriptor;
  char inotify_buffer[1024];

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = 2 * pow(10, -5);
};

#endif //EEG_PROCESSOR_DECIDER_H
