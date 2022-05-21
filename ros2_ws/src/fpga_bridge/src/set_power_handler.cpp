#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/set_power.hpp"

#include "NiFpga_board_control.h"

NiFpga_Session session;
NiFpga_Status status;
bool fpga_opened = false;

bool init_fpga(void) {
  /* must be called before any other calls */
  status = NiFpga_Initialize();
  if (NiFpga_IsError(status)) {
    RCLCPP_INFO(rclcpp::get_logger("set_power_handler"), "FPGA could not be initialized, exiting.");
    return false;
  }

  /* opens a session, downloads the bitstream, and runs the FPGA */

  /* TODO: Remove hardcoded bitfile. */
  NiFpga_MergeStatus(&status, NiFpga_Open("C:\\Users\\mTMS\\mtms\\bitfiles\\NiFpga_board_control_0_1_3.lvbitx",
          NiFpga_board_control_Signature,
          "PXI1Slot4",
          NiFpga_OpenAttribute_NoRun,
          &session));

  if (NiFpga_IsError(status)) {
      RCLCPP_INFO(rclcpp::get_logger("set_power_handler"), "FPGA bitfile could not be loaded, exiting.");
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

class SetPower : public rclcpp::Node
{
  public:
    SetPower()
    : Node("set_power_handler")
    {
      set_power_service_ = this->create_service<fpga_interfaces::srv::SetPower>("/fpga/set_power", set_power);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SetPower>::SharedPtr set_power_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("set_power_handler"), "Set power handler ready.");

  rclcpp::spin(std::make_shared<SetPower>());
  rclcpp::shutdown();

  close_fpga();
}
