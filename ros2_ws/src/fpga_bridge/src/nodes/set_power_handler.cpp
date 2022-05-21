#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/set_power.hpp"

#include "NiFpga_board_control.h"
#include "fpga.h"

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
