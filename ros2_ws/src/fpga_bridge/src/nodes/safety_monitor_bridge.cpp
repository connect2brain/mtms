#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/safety_monitor_errors.hpp"
#include "fpga_interfaces/msg/safety_monitor_state.hpp"
#include "fpga_interfaces/msg/version.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)

using namespace std::chrono_literals;

class SafetyMonitorBridge : public rclcpp::Node
{
  public:
    SafetyMonitorBridge()
    : Node("safety_monitor_bridge")
    {
      safety_monitor_publisher_ = this->create_publisher<fpga_interfaces::msg::SafetyMonitorState>("/fpga/safety_monitor_state", 10);
      timer_ = this->create_wall_timer(20ms, std::bind(&SafetyMonitorBridge::publish_safety_monitor_state, this));
    }

  private:
    fpga_interfaces::msg::SafetyMonitorErrors safety_monitor_error_integer_to_msg(uint16_t error_integer)
    {
      auto error_msg = fpga_interfaces::msg::SafetyMonitorErrors();

      error_msg.heartbeat = CHECK_BIT(error_integer, 0);
      error_msg.latched_fault = CHECK_BIT(error_integer, 1);
      error_msg.powersupply = CHECK_BIT(error_integer, 2);
      error_msg.safety_bus = CHECK_BIT(error_integer, 3);
      error_msg.coil = CHECK_BIT(error_integer, 4);
      error_msg.emergency_button = CHECK_BIT(error_integer, 5);
      error_msg.door = CHECK_BIT(error_integer, 6);
      error_msg.charger_overvoltage = CHECK_BIT(error_integer, 7);
      error_msg.charger_overtemperature = CHECK_BIT(error_integer, 8);
      error_msg.comparator = CHECK_BIT(error_integer, 9);
      error_msg.patient_safety = CHECK_BIT(error_integer, 10);
      error_msg.device_safety = CHECK_BIT(error_integer, 11);
      error_msg.charger_powerup = CHECK_BIT(error_integer, 12);
      error_msg.opto = CHECK_BIT(error_integer, 13);

      return error_msg;
    }

    void publish_safety_monitor_state()
    {
        /*
      uint8_t version_major = 0xFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU8(session,
                                        NiFpga_board_control_IndicatorU8_safety_monitor__version_major,
                                        &version_major));

      uint8_t version_minor = 0xFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU8(session,
                                        NiFpga_board_control_IndicatorU8_safety_monitor__version_minor,
                                        &version_minor));

      uint8_t version_patch = 0xFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU8(session,
                                        NiFpga_board_control_IndicatorU8_safety_monitor__version_patch,
                                        &version_patch));

      uint16_t cumulative_errors_integer = 0xFFFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU16(session,
                                         NiFpga_board_control_IndicatorU16_safety_monitor__cumulative_errors,
                                         &cumulative_errors_integer));

      uint16_t current_errors_integer = 0xFFFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU16(session,
                                         NiFpga_board_control_IndicatorU16_safety_monitor__current_errors,
                                         &current_errors_integer));

      uint16_t emergency_errors_integer = 0xFFFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU16(session,
                                         NiFpga_board_control_IndicatorU16_safety_monitor__emergency_errors,
                                         &emergency_errors_integer));

      uint32_t n_status_messages = 0xFFFFFFFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU32(session,
                                         NiFpga_board_control_IndicatorU32_safety_monitor__n_status_messages,
                                         &n_status_messages));

      uint32_t n_startup_messages = 0xFFFFFFFF;
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadU32(session,
                                         NiFpga_board_control_IndicatorU32_safety_monitor__n_startup_messages,
                                         &n_startup_messages));

      auto cumulative_errors = safety_monitor_error_integer_to_msg(cumulative_errors_integer);
      auto current_errors = safety_monitor_error_integer_to_msg(current_errors_integer);
      auto emergency_errors = safety_monitor_error_integer_to_msg(emergency_errors_integer);

      auto safety_monitor_state = fpga_interfaces::msg::SafetyMonitorState();

      safety_monitor_state.cumulative_errors = cumulative_errors;
      safety_monitor_state.current_errors = current_errors;
      safety_monitor_state.emergency_errors = emergency_errors;

      safety_monitor_state.version.major = version_major;
      safety_monitor_state.version.minor = version_minor;
      safety_monitor_state.version.patch = version_patch;

      safety_monitor_state.n_startup_messages = n_startup_messages;
      safety_monitor_state.n_status_messages = n_status_messages;


//      RCLCPP_INFO(this->get_logger(), "Publishing safety monitor state.");
      safety_monitor_publisher_->publish(safety_monitor_state);
         */
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<fpga_interfaces::msg::SafetyMonitorState>::SharedPtr safety_monitor_publisher_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);
  auto node = std::make_shared<SafetyMonitorBridge>();
  RCLCPP_INFO(rclcpp::get_logger("safety_monitor_bridge"), "Safety monitor bridge ready.");
#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif
  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
