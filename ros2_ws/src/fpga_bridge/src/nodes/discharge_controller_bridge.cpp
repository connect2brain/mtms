#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/discharge_controller_errors.hpp"
#include "fpga_interfaces/msg/discharge_controller_state.hpp"
#include "fpga_interfaces/msg/discharge_controller_states.hpp"
#include "fpga_interfaces/msg/version.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)

using namespace std::chrono_literals;

class DischargeControllerBridge : public rclcpp::Node
{
  public:
    DischargeControllerBridge()
    : Node("discharge_controller_bridge")
    {
      discharge_controllers_publisher_ = this->create_publisher<fpga_interfaces::msg::DischargeControllerStates>("/fpga/discharge_controller_states", 10);
      timer_ = this->create_wall_timer(20ms, std::bind(&DischargeControllerBridge::publish_discharge_controller_state, this));
    }

  private:
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

    void publish_discharge_controller_state()
    {
        /*
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
*/
      /* Arbitrarily take the number of discharge controllers to be the number of capacitor voltages reported by the FPGA;
         any other variable could be used here, as well. */
      /*
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

//      RCLCPP_INFO(this->get_logger(), "Publishing discharge controller state.");
      discharge_controllers_publisher_->publish(states);
       */
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<fpga_interfaces::msg::DischargeControllerStates>::SharedPtr discharge_controllers_publisher_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("discharge_controller_bridge"), "Discharge controller bridge ready.");

  rclcpp::spin(std::make_shared<DischargeControllerBridge>());
  rclcpp::shutdown();

  close_fpga();
}
