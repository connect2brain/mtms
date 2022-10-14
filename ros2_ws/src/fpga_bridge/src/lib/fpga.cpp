#include "rclcpp/rclcpp.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"

NiFpga_Session session;
NiFpga_Status status;
bool fpga_opened = false;

bool init_fpga() {
  /* must be called before any other calls */
  status = NiFpga_Initialize();
  if (NiFpga_IsError(status)) {
    RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "FPGA could not be initialized, exiting.");
    return false;
  }

  /* opens a session, downloads the bitstream, and runs the FPGA */

  RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "Opening FPGA.");

  auto bitfile = std::getenv("FPGA_BRIDGE_BITFILE");
  std::string bitfile_path;
  if (bitfile) {
    bitfile_path = "/app/ros2_ws/bitfiles/" + std::string(bitfile);
  } else {
    //default bitfile path
    bitfile_path = "/home/mtms/workspace/mtms/bitfiles/NiFpga_mtms_0_3_0.lvbitx";
  }

  NiFpga_MergeStatus(&status, NiFpga_Open(
      bitfile_path.c_str(),
      NiFpga_mTMS_Signature,
      "PXI1Slot4",
      NiFpga_OpenAttribute_NoRun,
      &session));

  if (NiFpga_IsError(status)) {
    RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "FPGA bitfile could not be loaded, exiting. Status: %d", status);
    return false;
  }

  RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "FPGA initialized.");

  fpga_opened = true;
  return true;
}

bool close_fpga() {
  RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "Closing FPGA.");

  if (fpga_opened) {
    /* must close if we successfully opened */
    NiFpga_MergeStatus(&status, NiFpga_Close(session, 0));
  }

  /* must be called after all other calls */
  NiFpga_MergeStatus(&status, NiFpga_Finalize());

  return true;
}

bool run_fpga() {
  RCLCPP_INFO(rclcpp::get_logger("run_fpga"), "Running FPGA.");

  NiFpga_MergeStatus(&status, NiFpga_Run(session, NiFpga_RunAttribute_WaitUntilDone));

  return true;
}
