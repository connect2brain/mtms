#include <chrono>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "fpga_interfaces/srv/set_power.hpp"

#include "fpga_interfaces/msg/safety_monitor_errors.hpp"
#include "fpga_interfaces/msg/safety_monitor_state.hpp"
#include "fpga_interfaces/msg/discharge_controller_errors.hpp"
#include "fpga_interfaces/msg/discharge_controller_state.hpp"
#include "fpga_interfaces/msg/discharge_controller_states.hpp"
#include "fpga_interfaces/msg/version.hpp"

#include "NiFpga_board_control.h"

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)

using namespace std::chrono_literals;
using std::placeholders::_1;

NiFpga_Session session;
NiFpga_Status status;
bool fpga_opened = false;

bool init_fpga(void) {
  /* must be called before any other calls */
  status = NiFpga_Initialize();
  if (NiFpga_IsError(status)) {
    RCLCPP_INFO(rclcpp::get_logger("fpga_bridge"), "FPGA could not be initialized, exiting.");
    return false;
  }

  /* opens a session, downloads the bitstream, and runs the FPGA */

  /* TODO: Remove hardcoded bitfile. */
  NiFpga_MergeStatus(&status, NiFpga_Open("C:\\Users\\mTMS\\mtms\\bitfiles\\NiFpga_board_control_0_1_0.lvbitx",
          NiFpga_board_control_Signature,
          "PXI1Slot4",
          0,
          &session));

  if (NiFpga_IsError(status)) {
      RCLCPP_INFO(rclcpp::get_logger("fpga_bridge"), "FPGA bitfile could not be loaded, exiting.");
      return false;
  }

  fpga_opened = true;
  return true;
}

bool close_fpga(void) {
  if (fpga_opened) {
    /* must close if we successfully opened */
    NiFpga_MergeStatus(&status, NiFpga_Close(session, 0));
  }

  /* must be called after all other calls */
  NiFpga_MergeStatus(&status, NiFpga_Finalize());

  return true;
}

void set_power(const std::shared_ptr<fpga_interfaces::srv::SetPower::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SetPower::Response> response)
{
  bool state = request->state;
  NiFpga_MergeStatus(&status,
                      NiFpga_WriteI16(session,
                                      NiFpga_board_control_ControlBool_signal__15_v_igbts,
                                      state));
  NiFpga_MergeStatus(&status,
                      NiFpga_WriteI16(session,
                                      NiFpga_board_control_ControlBool_signal__15_v_others,
                                      state));
  NiFpga_MergeStatus(&status,
                      NiFpga_WriteI16(session,
                                      NiFpga_board_control_ControlBool_signal__24_v,
                                      state));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("fpga"), "Set the board power to %s\n", state ? "true" : "false");
}

class FPGABridge : public rclcpp::Node
{
  public:
    FPGABridge()
    : Node("fpga_bridge")
    {
      safety_monitor_publisher_ = this->create_publisher<fpga_interfaces::msg::SafetyMonitorState>("/fpga/safety_monitor_state", 10);
      discharge_controllers_publisher_ = this->create_publisher<fpga_interfaces::msg::DischargeControllerStates>("/fpga/discharge_controller_states", 10);
      service_ = this->create_service<fpga_interfaces::srv::SetPower>("/fpga/set_power", set_power);
      timer_ = this->create_wall_timer(20ms, std::bind(&FPGABridge::timer_callback, this));
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

    fpga_interfaces::msg::DischargeControllerErrors discharge_controller_error_integer_to_msg(uint16_t error_integer)
    {
      auto error_msg = fpga_interfaces::msg::DischargeControllerErrors();

      error_msg.overvoltage = CHECK_BIT(error_integer, 0);
      error_msg.emergency_discharge_backup = CHECK_BIT(error_integer, 1);
      error_msg.safety_bus = CHECK_BIT(error_integer, 2);
      error_msg.powersupply = CHECK_BIT(error_integer, 3);
      error_msg.safety_bus_startup = CHECK_BIT(error_integer, 4);
      error_msg.acceptable_voltage_not_reached_startup = CHECK_BIT(error_integer, 5);
      error_msg.maximum_safe_voltage_exceeded_startup = CHECK_BIT(error_integer, 6);

      return error_msg;
    }

    void publish_safety_monitor_state()
    {
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

      RCLCPP_INFO(this->get_logger(), "Publishing safety monitor state.");
      safety_monitor_publisher_->publish(safety_monitor_state);
    }

    void publish_discharge_controller_state()
    {
      uint16_t capacitor_voltages[NiFpga_board_control_IndicatorArrayU16Size_discharge_controller__capacitor_voltage] = {0xFFFF};
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadArrayU16(session,
                                              NiFpga_board_control_IndicatorArrayU16_discharge_controller__capacitor_voltage,
                                              capacitor_voltages,
                                              NiFpga_board_control_IndicatorArrayU16Size_discharge_controller__capacitor_voltage));

      uint16_t errors[NiFpga_board_control_IndicatorArrayU16Size_discharge_controller__errors] = {0xFFFF};
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadArrayU16(session,
                                              NiFpga_board_control_IndicatorArrayU16_discharge_controller__errors,
                                              errors,
                                              NiFpga_board_control_IndicatorArrayU16Size_discharge_controller__errors));

      uint32_t n_startup_messages[NiFpga_board_control_IndicatorArrayU32Size_discharge_controller__n_startup_messages] = {0xFFFFFFFF};
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadArrayU32(session,
                                              NiFpga_board_control_IndicatorArrayU32_discharge_controller__n_startup_messages,
                                              n_startup_messages,
                                              NiFpga_board_control_IndicatorArrayU32Size_discharge_controller__n_startup_messages));

      uint32_t n_status_messages[NiFpga_board_control_IndicatorArrayU32Size_discharge_controller__n_status_messages] = {0xFFFFFFFF};
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadArrayU32(session,
                                              NiFpga_board_control_IndicatorArrayU32_discharge_controller__n_status_messages,
                                              n_status_messages,
                                              NiFpga_board_control_IndicatorArrayU32Size_discharge_controller__n_status_messages));

      uint8_t version_major[NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_major] = {0xFF};
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadArrayU8(session,
                                             NiFpga_board_control_IndicatorArrayU8_discharge_controller__version_major,
                                             version_major,
                                             NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_major));

      uint8_t version_minor[NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_minor] = {0xFF};
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadArrayU8(session,
                                             NiFpga_board_control_IndicatorArrayU8_discharge_controller__version_minor,
                                             version_minor,
                                             NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_minor));

      uint8_t version_patch[NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_patch] = {0xFF};
      NiFpga_MergeStatus(&status,
                          NiFpga_ReadArrayU8(session,
                                             NiFpga_board_control_IndicatorArrayU8_discharge_controller__version_patch,
                                             version_patch,
                                             NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_patch));

      auto states = fpga_interfaces::msg::DischargeControllerStates();

      /* Arbitrarily take the number of discharge controllers to be the number of capacitor voltages reported by the FPGA;
         any other variable could be used here, as well. */
      for (uint8_t i = 0; i < NiFpga_board_control_IndicatorArrayU16Size_discharge_controller__capacitor_voltage; i++) {
        auto state = fpga_interfaces::msg::DischargeControllerState();

        state.version.major = version_major[i];
        state.version.minor = version_minor[i];
        state.version.patch = version_patch[i];
        state.errors = discharge_controller_error_integer_to_msg(errors[i]);
        state.capacitor_voltage = capacitor_voltages[i];
        state.n_startup_messages = n_startup_messages[i];
        state.n_status_messages = n_status_messages[i];

        states.states.push_back(state);
      }

      RCLCPP_INFO(this->get_logger(), "Publishing discharge controller state.");
      discharge_controllers_publisher_->publish(states);
    }

    void timer_callback()
    {
      publish_safety_monitor_state();
      publish_discharge_controller_state();
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<fpga_interfaces::msg::SafetyMonitorState>::SharedPtr safety_monitor_publisher_;
    rclcpp::Publisher<fpga_interfaces::msg::DischargeControllerStates>::SharedPtr discharge_controllers_publisher_;
    rclcpp::Service<fpga_interfaces::srv::SetPower>::SharedPtr service_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("fpga_bridge"), "FPGA bridge ready.");

  rclcpp::spin(std::make_shared<FPGABridge>());
  rclcpp::shutdown();

  close_fpga();
}
