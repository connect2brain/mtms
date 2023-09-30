#include "rclcpp/rclcpp.hpp"

#include "mtms_device_interfaces/srv/start_session.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

void start_session([[maybe_unused]] const std::shared_ptr<mtms_device_interfaces::srv::StartSession::Request> request,
                      std::shared_ptr<mtms_device_interfaces::srv::StartSession::Response> response) {

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Startsession, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("start_session_handler"), "Started session");
}

class StartSession : public rclcpp::Node {
public:
  StartSession()
      : Node("start_session") {
    start_session_service_ = this->create_service<mtms_device_interfaces::srv::StartSession>("/mtms_device/start_session",
                                                                                            start_session);
  }

private:
  rclcpp::Service<mtms_device_interfaces::srv::StartSession>::SharedPtr start_session_service_;
};

int main(int argc, char **argv) {
  init_fpga();
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("start_session_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StartSession>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("start_session_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("start_session_handler"), "Start session handler ready.");

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
