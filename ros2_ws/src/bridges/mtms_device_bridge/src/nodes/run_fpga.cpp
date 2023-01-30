#include "rclcpp/rclcpp.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  run_fpga();

  close_fpga();
}
