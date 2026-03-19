#include "rclcpp/rclcpp.hpp"

#include "std_srvs/srv/trigger.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void stop_device([[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
                 std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("stop_device_handler"), "FPGA not in OK state during service call");
    response->success = false;
    return;
  }

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Stopdevice, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("stop_device_handler"), "Stopped device");
}

class StopDevice : public rclcpp::Node {
public:
  StopDevice()
      : Node("stop_device") {
    stop_device_service_ = this->create_service<std_srvs::srv::Trigger>("/mtms/device/stop", stop_device);
  }

private:
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_device_service_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<StopDevice>();

  RCLCPP_INFO(rclcpp::get_logger("stop_device_handler"), "Stop device handler ready.");

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
