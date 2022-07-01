#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/start_experiment.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void start_experiment(const std::shared_ptr<fpga_interfaces::srv::StartExperiment::Request> request,
          std::shared_ptr<fpga_interfaces::srv::StartExperiment::Response> response)
{

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Startexperiment, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("fpga"), "Started experiment");
}

class StartExperiment : public rclcpp::Node
{
  public:
    StartExperiment()
    : Node("start_experiment")
    {
      set_power_service_ = this->create_service<fpga_interfaces::srv::StartExperiment>("/fpga/start_experiment", start_experiment);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::StartExperiment>::SharedPtr set_power_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("set_power_handler"), "Set power handler ready.");

  rclcpp::spin(std::make_shared<StartExperiment>());
  rclcpp::shutdown();

  close_fpga();
}
