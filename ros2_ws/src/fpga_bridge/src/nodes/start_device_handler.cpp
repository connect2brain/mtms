#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/set_power.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void start_device(const std::shared_ptr<fpga_interfaces::srv::SetPower::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SetPower::Response> response)
{

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Startdevice, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("fpga"), "Set power to true");
}

class StartDevice : public rclcpp::Node
{
  public:
    StartDevice()
    : Node("start_device")
    {
      set_power_service_ = this->create_service<fpga_interfaces::srv::SetPower>("/fpga/start_device", start_device);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SetPower>::SharedPtr set_power_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("set_power_handler"), "Set power handler ready.");

  rclcpp::spin(std::make_shared<StartDevice>());
  rclcpp::shutdown();

  close_fpga();
}
