#include "rclcpp/rclcpp.hpp"

#include "fpga.h"
#include "NiFpga_board_control.h"

int main(int argc, char **argv)
{
  std::string bitfile = "C:\\Users\\mTMS\\mtms\\bitfiles\\NiFpga_board_control_0_1_3.lvbitx";
  std::string signature = "5A49B1B8A081F9837F478496550145CE";


  if (argc == 3) {

  }

  if (!init_fpga())
  {
    return 1;
  }

  run_fpga();

  close_fpga();
}
