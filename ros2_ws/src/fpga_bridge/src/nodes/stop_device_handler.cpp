#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/stop_device.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void stop_device(const std::shared_ptr<fpga_interfaces::srv::StopDevice::Request> request,
          std::shared_ptr<fpga_interfaces::srv::StopDevice::Response> response)
{

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Stopdevice, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("fpga"), "Stopped device");
}

class StopDevice : public rclcpp::Node
{
  public:
    StopDevice()
    : Node("stop_device")
    {
      set_power_service_ = this->create_service<fpga_interfaces::srv::StopDevice>("/fpga/stop_device", stop_device);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::StopDevice>::SharedPtr set_power_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("set_power_handler"), "Set power handler ready.");

  rclcpp::spin(std::make_shared<StopDevice>());
  rclcpp::shutdown();

  close_fpga();
}
