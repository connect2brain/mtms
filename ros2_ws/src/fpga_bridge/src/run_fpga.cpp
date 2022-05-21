#include "rclcpp/rclcpp.hpp"

#include "NiFpga_board_control.h"

NiFpga_Session session;
NiFpga_Status status;
bool fpga_opened = false;

bool init_fpga(void) {
  /* must be called before any other calls */
  status = NiFpga_Initialize();
  if (NiFpga_IsError(status)) {
    RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "FPGA could not be initialized, exiting.");
    return false;
  }

  /* opens a session, downloads the bitstream, and runs the FPGA */

  RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "Opening FPGA.");

  /* TODO: Remove hardcoded bitfile. */
  NiFpga_MergeStatus(&status, NiFpga_Open("C:\\Users\\mTMS\\mtms\\bitfiles\\NiFpga_board_control_0_1_3.lvbitx",
          NiFpga_board_control_Signature,
          "PXI1Slot4",
          NiFpga_OpenAttribute_NoRun,
          &session));

  if (NiFpga_IsError(status)) {
      RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "FPGA bitfile could not be loaded, exiting.");
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

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "FPGA initialized.");

  NiFpga_MergeStatus(&status, NiFpga_Run(session, NiFpga_RunAttribute_WaitUntilDone));

  RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "Closing FPGA.");

  close_fpga();
}
