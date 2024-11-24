//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_TRIGGERTIMER_H
#define EEG_PROCESSOR_TRIGGERTIMER_H

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/bool.hpp"

#include "eeg_interfaces/msg/sample.hpp"

#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

#include "system_interfaces/msg/timed_trigger.hpp"
#include "system_interfaces/srv/request_timed_trigger.hpp"

class TriggerTimer : public rclcpp::Node {
public:
  TriggerTimer();
  ~TriggerTimer();

private:
  rclcpp::Logger logger;
  rclcpp::Subscription<system_interfaces::msg::Healthcheck>::SharedPtr mtms_device_healthcheck_subscriber;
  rclcpp::Service<system_interfaces::srv::RequestTimedTrigger>::SharedPtr trigger_request_service;
  rclcpp::Subscription<eeg_interfaces::msg::Sample>::SharedPtr eeg_raw_subscriber;
  rclcpp::TimerBase::SharedPtr timer;

  int labjack_handle = -1;
  bool mtms_device_available = false;

  double_t last_latency_measurement_time = 0.0;

  /* Priority queue for trigger times. */
  std::priority_queue<double_t, std::vector<double_t>, std::greater<double_t>> trigger_queue;
  std::mutex queue_mutex;

  void attempt_labjack_connection();
  void handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg);
  void handle_eeg_raw(const std::shared_ptr<eeg_interfaces::msg::Sample> msg);
  void request_timed_trigger(
    const std::shared_ptr<system_interfaces::srv::RequestTimedTrigger::Request> request,
    std::shared_ptr<system_interfaces::srv::RequestTimedTrigger::Response> response);
  void trigger_labjack(const char* name);
  bool safe_error_check(int err, const char* action);

  /* Healthcheck */
  uint8_t status;
  std::string status_message;
  std::string actionable_message;
};

#endif //EEG_PROCESSOR_TRIGGERTIMER_H
