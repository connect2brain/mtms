#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/stop_device.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

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
      stop_device_service_ = this->create_service<fpga_interfaces::srv::StopDevice>("/fpga/stop_device", stop_device);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::StopDevice>::SharedPtr stop_device_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);
  auto node = std::make_shared<StopDevice>();
  RCLCPP_INFO(rclcpp::get_logger("stop_device_handler"), "Stop device handler ready.");
#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif
  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
