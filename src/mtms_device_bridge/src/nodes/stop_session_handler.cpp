#include "rclcpp/rclcpp.hpp"

#include "system_interfaces/srv/stop_session.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void stop_session([[maybe_unused]] const std::shared_ptr<system_interfaces::srv::StopSession::Request> request,
                     std::shared_ptr<system_interfaces::srv::StopSession::Response> response) {
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("stop_session_handler"), "FPGA not in OK state during service call");
    response->success = false;
    return;
  }

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Stopsession, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("stop_session_handler"), "Stopped session");
}

class StopSession : public rclcpp::Node {
public:
  StopSession()
      : Node("stop_session") {
    stop_session_service_ = this->create_service<system_interfaces::srv::StopSession>("/mtms/device/session/stop",
                                                                                      stop_session);
  }

private:
  rclcpp::Service<system_interfaces::srv::StopSession>::SharedPtr stop_session_service_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<StopSession>();

  RCLCPP_INFO(rclcpp::get_logger("stop_session_handler"), "Stop session handler ready.");

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
