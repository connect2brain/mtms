#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/stop_experiment.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void stop_experiment(const std::shared_ptr<fpga_interfaces::srv::StopExperiment::Request> request,
          std::shared_ptr<fpga_interfaces::srv::StopExperiment::Response> response)
{

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Stopexperiment, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("fpga"), "Stoped experiment");
}

class StopExperiment : public rclcpp::Node
{
  public:
    StopExperiment()
    : Node("stop_experiment")
    {
      set_power_service_ = this->create_service<fpga_interfaces::srv::StopExperiment>("/fpga/stop_experiment", stop_experiment);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::StopExperiment>::SharedPtr set_power_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("set_power_handler"), "Set power handler ready.");

  rclcpp::spin(std::make_shared<StopExperiment>());
  rclcpp::shutdown();

  close_fpga();
}
