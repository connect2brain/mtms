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

  auto bitfile = std::getenv("BITFILE");
  auto bitfile_directory = std::getenv("BITFILE_DIRECTORY");
  auto bitfile_signature = std::getenv("BITFILE_SIGNATURE");

  if (!bitfile || !bitfile_directory) {
    RCLCPP_ERROR(rclcpp::get_logger("run_fpga"),
                 "No BITFILE or BITFILE_DIRECTORY environment variable set.");
    return false;
  }
  if (!bitfile_signature) {
    RCLCPP_ERROR(rclcpp::get_logger("run_fpga"),
                 "No BITFILE_SIGNATURE environment variable set.");
    return false;
  }

  std::string bitfile_path_str = std::string(bitfile_directory) + "/" + std::string(bitfile);
  std::string bitfile_signature_str = std::string(bitfile_signature);

  NiFpga_MergeStatus(&status, NiFpga_Open(
      bitfile_path_str.c_str(),
      bitfile_signature_str.c_str(),
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
