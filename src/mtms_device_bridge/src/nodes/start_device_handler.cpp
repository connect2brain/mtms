#include "rclcpp/rclcpp.hpp"

#include "std_srvs/srv/trigger.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void start_device([[maybe_unused]] const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
                  std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("start_device_handler"), "FPGA not in OK state during service call");
    response->success = false;
    return;
  }

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Startdevice, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("start_device_handler"), "Started device");
}

class StartDevice : public rclcpp::Node {
public:
  StartDevice()
      : Node("start_device") {
    start_device_service_ = this->create_service<std_srvs::srv::Trigger>("/mtms/device/start", start_device);
  }

private:
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr start_device_service_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<StartDevice>();

  RCLCPP_INFO(rclcpp::get_logger("start_device_handler"), "Start device handler ready.");

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
