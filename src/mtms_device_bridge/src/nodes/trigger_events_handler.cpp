#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/empty.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void handle_request(
    [[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response) {

  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("trigger_events_handler"), "FPGA not in OK state while attempting to trigger events.");
    response->success = false;
    return;
  }

  NiFpga_MergeStatus(&status,
                     NiFpga_WriteBool(session,
                                      NiFpga_mTMS_ControlBool_Eventtrigger,
                                      true));

  RCLCPP_INFO(rclcpp::get_logger("trigger_events_handler"), "Events with execution condition WAIT_FOR_TRIGGER triggered.");
  response->success = true;
}

class TriggerEventsHandler : public rclcpp::Node {
public:
  TriggerEventsHandler() : Node("trigger_events_handler") {
    trigger_service_ = this->create_service<std_srvs::srv::Trigger>(
        "/mtms/device/events/trigger", &handle_request);

    const std::string heartbeat_topic = "/mtms/device/heartbeat";
    auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(heartbeat_topic, 10);
    this->create_wall_timer(std::chrono::milliseconds(500), [heartbeat_publisher]() {
      heartbeat_publisher->publish(std_msgs::msg::Empty());
    });
  }

private:
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr trigger_service_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<TriggerEventsHandler>();

  RCLCPP_INFO(rclcpp::get_logger("trigger_events_handler"), "Trigger events handler ready.");

  auto timer = node->create_wall_timer(
      std::chrono::milliseconds(FPGA_OK_CHECK_INTERVAL_MS),
      [&]() {
          if (!is_fpga_ok()) {
              init_fpga();
          }
      }
  );
  rclcpp::spin(node);

  close_fpga();
  rclcpp::shutdown();
}
