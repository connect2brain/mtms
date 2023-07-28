//
// Created by alqio on 11.11.2022.
//
#include "scheduling_utils.h"
#include "memory_utils.h"
#include "rclcpp/rclcpp.hpp"
#include "neuronavigation_interfaces/srv/efield.hpp"
#include "neuronavigation_interfaces/srv/initialize_efield.hpp"
#include "neuronavigation_interfaces/srv/set_coil.hpp"
#include "neuronavigation_interfaces/srv/efield_norm.hpp"
#include "neuronavigation_interfaces/srv/efield_roi.hpp"
#include "neuronavigation_interfaces/srv/efield_roi_max.hpp"

#include "efield_estimation.h"

class EField : public rclcpp::Node {
public:
  EField() : Node("efield") {

    auto service_callback_efield_norm = [this](
        const std::shared_ptr<neuronavigation_interfaces::srv::EfieldNorm::Request> request,
        std::shared_ptr<neuronavigation_interfaces::srv::EfieldNorm::Response> response) -> void {

      RCLCPP_INFO(rclcpp::get_logger("efield_getnorm"), "Request received from /efield/get_norm");

      std::vector<float> position;
      std::vector<double> orientation;

      position.push_back(static_cast<float>(request->coordinate.position.x));
      position.push_back(static_cast<float>(request->coordinate.position.y));
      position.push_back(static_cast<float>(request->coordinate.position.z));
      orientation.push_back(request->coordinate.orientation.alpha);
      orientation.push_back(request->coordinate.orientation.beta);
      orientation.push_back(request->coordinate.orientation.gamma);

      //efield_estimation(position, request->transducer_rotation, response->efield_data, response->efield_data_column1,response->efield_data_column2, response->efield_data_column3);
      efield_estimation(position, request->transducer_rotation, response->efield_norm);

    };

    auto service_callback_efield_vector = [this](
              const std::shared_ptr<neuronavigation_interfaces::srv::Efield::Request> request,
              std::shared_ptr<neuronavigation_interfaces::srv::Efield::Response> response) -> void {

          RCLCPP_INFO(rclcpp::get_logger("efield_getefieldvector"), "Request received from /efield/get_efieldvector");

          std::vector<float> position;
          std::vector<double> orientation;

          position.push_back(static_cast<float>(request->coordinate.position.x));
          position.push_back(static_cast<float>(request->coordinate.position.y));
          position.push_back(static_cast<float>(request->coordinate.position.z));
          orientation.push_back(request->coordinate.orientation.alpha);
          orientation.push_back(request->coordinate.orientation.beta);
          orientation.push_back(request->coordinate.orientation.gamma);

          //efield_estimation(position, request->transducer_rotation, response->efield_data, response->efield_data_column1,response->efield_data_column2, response->efield_data_column3);
          efield_estimation_vector(position, request->transducer_rotation, response->efield_data.enorm, response->efield_data.column1, response->efield_data.column2, response->efield_data.column3);

      };

      auto service_callback_efield_vector_ROI=[this](const std::shared_ptr<neuronavigation_interfaces::srv::EfieldRoi::Request> request,
                                                     std::shared_ptr<neuronavigation_interfaces::srv::EfieldRoi::Response> response)-> void {
          RCLCPP_INFO(rclcpp::get_logger("efield-get_ROIefieldvector"), "Request received from /efield/get_ROIefieldvector");
          std::vector<float> position;
          std::vector<double> orientation;

          position.push_back(static_cast<float>(request->coordinate.position.x));
          position.push_back(static_cast<float>(request->coordinate.position.y));
          position.push_back(static_cast<float>(request->coordinate.position.z));

          efield_estimation_ROI(position, request->transducer_rotation, request->id_list,response->efield_data.enorm, response->efield_data.column1, response->efield_data.column2, response->efield_data.column3,response->efield_data.mvector, response->efield_data.maxindex);

      };

      auto service_callback_efield_vector_ROI_max_loc=[this](const std::shared_ptr<neuronavigation_interfaces::srv::EfieldRoiMax::Request> request,
                                                     std::shared_ptr<neuronavigation_interfaces::srv::EfieldRoiMax::Response> response)-> void {
          RCLCPP_INFO(rclcpp::get_logger("efield-get_ROIefieldvectorMax"), "Request received from /efield/get_ROIefieldvectorMax");
          std::vector<float> position;
          std::vector<double> orientation;

          position.push_back(static_cast<float>(request->coordinate.position.x));
          position.push_back(static_cast<float>(request->coordinate.position.y));
          position.push_back(static_cast<float>(request->coordinate.position.z));

          efield_estimation_ROI_max_loc(position, request->transducer_rotation, request->id_list,response->efield_data.enorm, response->efield_data.mvector, response->efield_data.maxindex);

      };

    auto service_callback_init_efield= [this](const std::shared_ptr<neuronavigation_interfaces::srv::InitializeEfield::Request> request,
        std::shared_ptr<neuronavigation_interfaces::srv::InitializeEfield::Response> response)-> void {

        RCLCPP_INFO(rclcpp::get_logger("efield_initialize"), "Request received from /efield/initialize");
        init_efield(request->cortex_model_path, request->mesh_models_paths,request->conductivities_inside, request->conductivities_outside, response->success);
        set_coil(request->coil_model_path, request->coil_set, response->success);
    };

    auto service_callback_set_coil=[this](const std::shared_ptr<neuronavigation_interfaces::srv::SetCoil::Request> request,
                                  std::shared_ptr<neuronavigation_interfaces::srv::SetCoil::Response> response)-> void {
        RCLCPP_INFO(rclcpp::get_logger("efield_setcoil"), "Request received from /efield/set_coil");

        set_coil(request->coil_model_path, request->coil_set, response->success);

    };



      efield_service_init = this->create_service<neuronavigation_interfaces::srv::InitializeEfield>("/efield/initialize", service_callback_init_efield);
      efield_service_enorm = this->create_service<neuronavigation_interfaces::srv::EfieldNorm>("/efield/get_norm", service_callback_efield_norm);
      efield_service_coil = this->create_service<neuronavigation_interfaces::srv::SetCoil>("/efield/set_coil", service_callback_set_coil);
      efield_service_vectorefield = this->create_service<neuronavigation_interfaces::srv::Efield>("/efield/get_efieldvector", service_callback_efield_vector);
      efield_service_vectorfield_ROI = this->create_service<neuronavigation_interfaces::srv::EfieldRoi>("/efield/get_ROIefieldvector", service_callback_efield_vector_ROI);
      efield_service_vectorfield_ROI_max_loc = this->create_service<neuronavigation_interfaces::srv::EfieldRoiMax>("/efield/get_ROIefieldvectorMax", service_callback_efield_vector_ROI_max_loc);

  }

private:
  rclcpp::Service<neuronavigation_interfaces::srv::EfieldNorm>::SharedPtr efield_service_enorm;
  rclcpp::Service<neuronavigation_interfaces::srv::InitializeEfield>::SharedPtr efield_service_init;
  rclcpp::Service<neuronavigation_interfaces::srv::SetCoil>::SharedPtr efield_service_coil;
  rclcpp::Service<neuronavigation_interfaces::srv::Efield>::SharedPtr efield_service_vectorefield;
  rclcpp::Service<neuronavigation_interfaces::srv::EfieldRoi>::SharedPtr efield_service_vectorfield_ROI;
  rclcpp::Service<neuronavigation_interfaces::srv::EfieldRoiMax>::SharedPtr efield_service_vectorfield_ROI_max_loc;

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
