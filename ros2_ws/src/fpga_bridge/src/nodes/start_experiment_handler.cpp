#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/start_experiment.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

void start_experiment(const std::shared_ptr<fpga_interfaces::srv::StartExperiment::Request> request,
                      std::shared_ptr<fpga_interfaces::srv::StartExperiment::Response> response) {

  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Startexperiment, true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("fpga"), "Started experiment");
}

class StartExperiment : public rclcpp::Node {
public:
  StartExperiment()
      : Node("start_experiment") {
    start_experiment_service_ = this->create_service<fpga_interfaces::srv::StartExperiment>("/fpga/start_experiment",
                                                                                            start_experiment);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::StartExperiment>::SharedPtr start_experiment_service_;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("start_experiment_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StartExperiment>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("start_experiment_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("start_experiment_handler"), "Start experiment handler ready.");


  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
