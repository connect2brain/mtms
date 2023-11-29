#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/executors/multi_threaded_executor.hpp"

#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"

#include "system_interfaces/msg/session.hpp"
#include "system_interfaces/msg/session_state.hpp"

#include "system_interfaces/srv/start_session.hpp"
#include "system_interfaces/srv/stop_session.hpp"


class SessionManager : public rclcpp::Node {
public:
  SessionManager();

private:
  void handle_mtms_device_session(const std::shared_ptr<system_interfaces::msg::Session> msg);

  void handle_start_session(
    const std::shared_ptr<system_interfaces::srv::StartSession::Request> request,
    std::shared_ptr<system_interfaces::srv::StartSession::Response> response);

  void handle_stop_session(
    const std::shared_ptr<system_interfaces::srv::StopSession::Request> request,
    std::shared_ptr<system_interfaces::srv::StopSession::Response> response);

  bool call_start_mtms_session();
  bool call_stop_mtms_session();

  rclcpp::Subscription<system_interfaces::msg::Session>::SharedPtr session_subscriber;
  rclcpp::Publisher<system_interfaces::msg::Session>::SharedPtr session_publisher;

  rclcpp::Service<system_interfaces::srv::StartSession>::SharedPtr start_session_service;
  rclcpp::Service<system_interfaces::srv::StopSession>::SharedPtr stop_session_service;

  rclcpp::Client<system_interfaces::srv::StartSession>::SharedPtr start_mtms_session_client;
  rclcpp::Client<system_interfaces::srv::StopSession>::SharedPtr stop_mtms_session_client;

  rclcpp::CallbackGroup::SharedPtr callback_group;
};

#endif //SESSION_MANAGER_H
