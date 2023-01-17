#include "rclcpp/rclcpp.hpp"

#include "mtms_device_interfaces/srv/stop_device.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

void stop_device(const std::shared_ptr<mtms_device_interfaces::srv::StopDevice::Request> request,
                 std::shared_ptr<mtms_device_interfaces::srv::StopDevice::Response> response) {

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Stopdevice, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("stop_device_handler"), "Stopped device");
}

class StopDevice : public rclcpp::Node {
public:
  StopDevice()
      : Node("stop_device") {
    stop_device_service_ = this->create_service<mtms_device_interfaces::srv::StopDevice>("/mtms_device/stop_device", stop_device);
  }

private:
  rclcpp::Service<mtms_device_interfaces::srv::StopDevice>::SharedPtr stop_device_service_;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("stop_device_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StopDevice>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("stop_device_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("stop_device_handler"), "Stop device handler ready.");

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
