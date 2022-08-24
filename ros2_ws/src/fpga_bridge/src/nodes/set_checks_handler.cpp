#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/set_checks.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void set_checks(const std::shared_ptr<fpga_interfaces::srv::SetChecks::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SetChecks::Response> response)
{
  auto enabled = request->enabled;

  NiFpga_MergeStatus(&status,
                     NiFpga_WriteBool(session,
                                      NiFpga_mTMS_ControlBool_Disablechecks,
                                      enabled));

  response->success = true;

  RCLCPP_INFO(rclcpp::get_logger("set_checks_handler"), "Set checks: %s", enabled ? "true" : "false");
}

class SetChecksHandler : public rclcpp::Node
{
  public:
    SetChecksHandler()
    : Node("set_checks_handler")
    {
      set_checks_service_ = this->create_service<fpga_interfaces::srv::SetChecks>("/fpga/set_checks", set_checks);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SetChecks>::SharedPtr set_checks_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("set_checks_handler"), "Set checks handler ready.");

  rclcpp::spin(std::make_shared<SetChecksHandler>());
  rclcpp::shutdown();

  close_fpga();
}
