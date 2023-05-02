//
// Created by alqio on 11.11.2022.
//
#include "scheduling_utils.h"
#include "memory_utils.h"
#include "rclcpp/rclcpp.hpp"
#include "neuronavigation_interfaces/srv/efield.hpp"
#include "neuronavigation_interfaces/srv/efield_init.hpp"
#include "neuronavigation_interfaces/srv/efield_coil.hpp"

#include "efield_estimation.h"
#include "EfieldInterface.h"

class EField : public rclcpp::Node {
public:
  EField() : Node("efield") {

    auto service_callback_efield_norm = [this](
        const std::shared_ptr<neuronavigation_interfaces::srv::Efield::Request> request,
        std::shared_ptr<neuronavigation_interfaces::srv::Efield::Response> response) -> void {

      RCLCPP_INFO(rclcpp::get_logger("efield_getnorm"), "Request received from efield norm");

      std::vector<float> position;
      std::vector<double> orientation;

      position.push_back(static_cast<float>(request->coordinate.position.x));
      position.push_back(static_cast<float>(request->coordinate.position.y));
      position.push_back(static_cast<float>(request->coordinate.position.z));
      orientation.push_back(request->coordinate.orientation.alpha);
      orientation.push_back(request->coordinate.orientation.beta);
      orientation.push_back(request->coordinate.orientation.gamma);

      efield_estimation(position, orientation, request->transducer_rotation, response->efield_data);

    };

    auto service_callback_init_efield= [this](const std::shared_ptr<neuronavigation_interfaces::srv::EfieldInit::Request> request,
        std::shared_ptr<neuronavigation_interfaces::srv::EfieldInit::Response> response)-> void {

        RCLCPP_INFO(rclcpp::get_logger("efield_init"), "Request received from init");
        init_efield(request->cortexfile, request->meshfile, request->ci, request->co, response->success);
        add_coil(request->coilfile, response->success);
    };

    auto service_callback_set_coil=[this](const std::shared_ptr<neuronavigation_interfaces::srv::EfieldCoil::Request> request,
                                  std::shared_ptr<neuronavigation_interfaces::srv::EfieldCoil::Response> response)-> void {
        RCLCPP_INFO(rclcpp::get_logger("efield_setcoil"), "Request received from coil change");

        add_coil(request->coilfile, response->success);

    };

      efield_service_init = this->create_service<neuronavigation_interfaces::srv::EfieldInit>("/efield/init", service_callback_init_efield);
      efield_service_enorm = this->create_service<neuronavigation_interfaces::srv::Efield>("/efield/getnorm", service_callback_efield_norm);
      efield_service_coil = this->create_service<neuronavigation_interfaces::srv::EfieldCoil>("/efield/setcoil", service_callback_set_coil);
  }

private:
  rclcpp::Service<neuronavigation_interfaces::srv::Efield>::SharedPtr efield_service_enorm;
  rclcpp::Service<neuronavigation_interfaces::srv::EfieldInit>::SharedPtr efield_service_init;
  rclcpp::Service<neuronavigation_interfaces::srv::EfieldCoil>::SharedPtr efield_service_coil;
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
