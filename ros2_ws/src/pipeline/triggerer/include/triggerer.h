//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_TRIGGERER_H
#define EEG_PROCESSOR_TRIGGERER_H

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/bool.hpp"

#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

#include "event_interfaces/msg/event_trigger.hpp"

class Triggerer : public rclcpp::Node {
public:
  Triggerer();
  ~Triggerer();

private:
  void attempt_labjack_connection();
  bool safe_error_check(int err, const char* action);

  void handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg);

  void process_event_trigger(const std::shared_ptr<event_interfaces::msg::EventTrigger> msg);

  rclcpp::Logger logger;
  rclcpp::TimerBase::SharedPtr timer;

  rclcpp::Subscription<event_interfaces::msg::EventTrigger>::SharedPtr event_trigger_subscriber;

  rclcpp::Subscription<system_interfaces::msg::Healthcheck>::SharedPtr mtms_device_healthcheck_subscriber;
  rclcpp::Publisher<event_interfaces::msg::EventTrigger>::SharedPtr event_trigger_publisher;

  /* LabJack connection */
  int labjack_handle = -1;

  /* Healthcheck */
  uint8_t status;
  std::string status_message;
  std::string actionable_message;

  /* mTMS device healthcheck */
  bool mtms_device_available = false;
};

#endif //EEG_PROCESSOR_TRIGGERER_H
