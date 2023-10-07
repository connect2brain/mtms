#include "rclcpp/rclcpp.hpp"

#include "mtms_device_interfaces/srv/allow_trigger_out.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

void allow_trigger_out([[maybe_unused]] const std::shared_ptr<mtms_device_interfaces::srv::AllowTriggerOut::Request> request,
                        std::shared_ptr<mtms_device_interfaces::srv::AllowTriggerOut::Response> response) {

  bool allow_trigger_out = request->allow_trigger_out;

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Allowtriggerout, allow_trigger_out));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("allow_trigger_out_handler"), "%s trigger out", allow_trigger_out ? "Allow" : "Disallow");
}

class AllowTriggerOut : public rclcpp::Node {
public:
  AllowTriggerOut()
      : Node("allow_trigger_out") {
    allow_trigger_out_service_ = this->create_service<mtms_device_interfaces::srv::AllowTriggerOut>("/mtms_device/allow_trigger_out", allow_trigger_out);
  }

private:
  rclcpp::Service<mtms_device_interfaces::srv::AllowTriggerOut>::SharedPtr allow_trigger_out_service_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("allow_trigger_out_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<AllowTriggerOut>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("allow_trigger_out_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("allow_trigger_out_handler"), "Allow trigger out handler ready.");

  init_fpga();

  while (rclcpp::ok()) {
    if (!is_fpga_ok()) {
      close_fpga();
      init_fpga();
    }
    rclcpp::spin_some(node);
  }
  close_fpga();
  rclcpp::shutdown();
}
