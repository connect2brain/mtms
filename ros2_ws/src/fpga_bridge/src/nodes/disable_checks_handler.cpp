#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/disable_checks.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void disable_checks(const std::shared_ptr<fpga_interfaces::srv::DisableChecks::Request> request,
          std::shared_ptr<fpga_interfaces::srv::DisableChecks::Response> response)
{
  auto enabled = request->enabled;

  NiFpga_MergeStatus(&status,
                     NiFpga_WriteBool(session,
                                      NiFpga_mTMS_ControlBool_Disablechecks,
                                      enabled));

  response->success = true;

  RCLCPP_INFO(rclcpp::get_logger("disable_checks_handler"), "Disable checks: %s", enabled ? "true" : "false");
}

class DisableChecksHandler : public rclcpp::Node
{
  public:
    DisableChecksHandler()
    : Node("disable_checks_handler")
    {
      disable_checks_service_ = this->create_service<fpga_interfaces::srv::DisableChecks>("/fpga/disable_checks", disable_checks);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::DisableChecks>::SharedPtr disable_checks_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("disable_checks_handler"), "Disable checks handler ready.");

  rclcpp::spin(std::make_shared<DisableChecksHandler>());
  rclcpp::shutdown();

  close_fpga();
}
