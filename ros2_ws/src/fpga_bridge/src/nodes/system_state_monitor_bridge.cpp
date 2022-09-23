#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/system_state.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

#define CHECK_BIT(var, pos) (((var)>>(pos)) & 1)

using namespace std::chrono_literals;

NiFpga_mTMS_IndicatorU8 system_state_register = NiFpga_mTMS_IndicatorU8_Systemstate;

class SystemStateMonitorBridge : public rclcpp::Node {
public:
  SystemStateMonitorBridge()
      : Node("system_state_monitor_bridge") {
    state_monitor_publisher_ = this->create_publisher<fpga_interfaces::msg::SystemState>(
        "/fpga/system_state_monitor_state", 10);
    timer_ = this->create_wall_timer(20ms, std::bind(&SystemStateMonitorBridge::publish_system_state, this));
  }

private:
  void publish_system_state() {
    uint8_t system_state = 0xFF;
    NiFpga_MergeStatus(&status,
                       NiFpga_ReadU8(session,
                                     system_state_register,
                                     &system_state));

    std::string system_state_str;
    switch (system_state) {
      case 0:
        system_state_str = "Not operational";
        break;
      case 1:
        system_state_str = "Startup";
        break;
      case 2:
        system_state_str = "Operational";
        break;
      case 3:
        system_state_str = "Shutdown";
        break;
      default:
        system_state_str = "Unknown state";
        break;
    }

    auto system_state_msg = fpga_interfaces::msg::SystemState();
    system_state_msg.state = system_state_str;
    state_monitor_publisher_->publish(system_state_msg);

    RCLCPP_DEBUG(rclcpp::get_logger("system_state_monitor_state"), "Publishing system state: %s",
                 system_state_str.c_str());

  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<fpga_interfaces::msg::SystemState>::SharedPtr state_monitor_publisher_;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("system_state_monitor_state"), "Setting thread scheduling and memory lock");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<SystemStateMonitorBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("system_state_monitor_state"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("system_state_monitor_state"), "System state monitor bridge ready.");


  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
