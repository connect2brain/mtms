#include "rclcpp/rclcpp.hpp"

#include "mtms_device_interfaces/srv/allow_trigger_out.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void allow_trigger_out([[maybe_unused]] const std::shared_ptr<mtms_device_interfaces::srv::AllowTriggerOut::Request> request,
                        std::shared_ptr<mtms_device_interfaces::srv::AllowTriggerOut::Response> response) {
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("allow_trigger_out"), "FPGA not in OK state during service call");
    response->success = false;
    return;
  }

  bool allow_trigger_out = request->allow_trigger_out;

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Allowtriggerout, allow_trigger_out));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("allow_trigger_out"), "%s trigger out.", allow_trigger_out ? "Allowing" : "Disallowing");
}

class AllowTriggerOut : public rclcpp::Node {
public:
  AllowTriggerOut() : Node("allow_trigger_out") {
    allow_trigger_out_service_ = this->create_service<mtms_device_interfaces::srv::AllowTriggerOut>("/mtms_device/allow_trigger_out", allow_trigger_out);
  }

private:
  rclcpp::Service<mtms_device_interfaces::srv::AllowTriggerOut>::SharedPtr allow_trigger_out_service_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<AllowTriggerOut>();
  RCLCPP_INFO(rclcpp::get_logger("allow_trigger_out_handler"), "Allow trigger out handler ready.");

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
