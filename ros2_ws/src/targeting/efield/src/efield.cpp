//
// Created by alqio on 11.11.2022.
//
#include "scheduling_utils.h"
#include "memory_utils.h"
#include "rclcpp/rclcpp.hpp"
#include "neuronavigation_interfaces/srv/efield.hpp"
#include "efield_estimation.h"


class EField : public rclcpp::Node {
public:
  EField() : Node("efield") {

    auto service_callback = [this](
        const std::shared_ptr<neuronavigation_interfaces::srv::Efield::Request> request,
        std::shared_ptr<neuronavigation_interfaces::srv::Efield::Response> response) -> void {

      RCLCPP_INFO(rclcpp::get_logger("efield"), "Request received");

      std::vector<float> position;
      std::vector<double> orientation;

      position.push_back(static_cast<float>(request->coordinate.position.x));
      position.push_back(static_cast<float>(request->coordinate.position.y));
      position.push_back(static_cast<float>(request->coordinate.position.z));
      orientation.push_back(request->coordinate.orientation.alpha);
      orientation.push_back(request->coordinate.orientation.beta);
      orientation.push_back(request->coordinate.orientation.gamma);

      std::vector<double> efield_vector;
      efield_estimation(position, orientation, request->transducer_rotation, response->efield_data);

    };


    efield_service = this->create_service<neuronavigation_interfaces::srv::Efield>("/efield", service_callback);

    init_efield();
  }

private:
  rclcpp::Service<neuronavigation_interfaces::srv::Efield>::SharedPtr efield_service;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("efield"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EField>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("efield"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
