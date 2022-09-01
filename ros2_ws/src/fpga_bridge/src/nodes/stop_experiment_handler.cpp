#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/stop_experiment.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

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
      stop_experiment_service_ = this->create_service<fpga_interfaces::srv::StopExperiment>("/fpga/stop_experiment", stop_experiment);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::StopExperiment>::SharedPtr stop_experiment_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);
  auto node = std::make_shared<StopExperiment>();
  RCLCPP_INFO(rclcpp::get_logger("stop_experiment_handler"), "Stop experiment handler ready.");
#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif
  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
