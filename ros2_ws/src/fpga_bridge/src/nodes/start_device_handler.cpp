#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/start_device.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void start_device(const std::shared_ptr<fpga_interfaces::srv::StartDevice::Request> request,
          std::shared_ptr<fpga_interfaces::srv::StartDevice::Response> response)
{

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Startdevice, true));

  response->success = true;
  RCLCPP_DEBUG(rclcpp::get_logger("fpga"), "Started device");
}

class StartDevice : public rclcpp::Node
{
  public:
    StartDevice()
    : Node("start_device")
    {
      start_device_service_ = this->create_service<fpga_interfaces::srv::StartDevice>("/fpga/start_device", start_device);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::StartDevice>::SharedPtr start_device_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("start_device_handler"), "Start device handler ready.");

  rclcpp::spin(std::make_shared<StartDevice>());
  rclcpp::shutdown();

  close_fpga();
}
