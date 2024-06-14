//
// Created by alqio on 16.1.2023.
//

#ifndef EEG_PROCESSOR_LABJACKTRIGGERER_H
#define EEG_PROCESSOR_LABJACKTRIGGERER_H

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/bool.hpp"

#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

#include "event_interfaces/msg/request_trigger.hpp"

class LabjackTriggerer : public rclcpp::Node {
public:
  LabjackTriggerer();
  ~LabjackTriggerer();

private:
  void attempt_labjack_connection();
  bool safe_error_check(int err, const char* action);

  void handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg);

  void request_trigger(const std::shared_ptr<event_interfaces::msg::RequestTrigger> msg);

  rclcpp::Logger logger;
  rclcpp::TimerBase::SharedPtr timer;

  rclcpp::Subscription<event_interfaces::msg::RequestTrigger>::SharedPtr trigger_request_subscriber;

  rclcpp::Subscription<system_interfaces::msg::Healthcheck>::SharedPtr mtms_device_healthcheck_subscriber;

  /* LabJack connection */
  int labjack_handle = -1;

  /* Healthcheck */
  uint8_t status;
  std::string status_message;
  std::string actionable_message;

  /* mTMS device healthcheck */
  bool mtms_device_available = false;
};

#endif //EEG_PROCESSOR_LABJACKTRIGGERER_H
