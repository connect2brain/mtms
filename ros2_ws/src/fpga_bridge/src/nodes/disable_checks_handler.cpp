#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/disable_checks.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

void disable_checks(const std::shared_ptr<fpga_interfaces::srv::DisableChecks::Request> request,
                    std::shared_ptr<fpga_interfaces::srv::DisableChecks::Response> response) {
  auto disabled = request->disabled;

  NiFpga_MergeStatus(&status,
                     NiFpga_WriteBool(session,
                                      NiFpga_mTMS_ControlBool_Disablechecks,
                                      disabled));

  response->success = true;

  RCLCPP_INFO(rclcpp::get_logger("disable_checks_handler"), "Disable checks: %s", disabled ? "true" : "false");
}

class DisableChecksHandler : public rclcpp::Node {
public:
  DisableChecksHandler()
      : Node("disable_checks_handler") {
    disable_checks_service_ = this->create_service<fpga_interfaces::srv::DisableChecks>("/fpga/disable_checks",
                                                                                        disable_checks);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::DisableChecks>::SharedPtr disable_checks_service_;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

  auto node = std::make_shared<DisableChecksHandler>();
  RCLCPP_INFO(rclcpp::get_logger("disable_checks_handler"), "Disable checks handler ready.");

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
