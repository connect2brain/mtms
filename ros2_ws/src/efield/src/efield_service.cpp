#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "neuronavigation_interfaces/msg/pose_using_euler_angles.hpp"

#include "neuronavigation_interfaces/srv/efield.hpp"

#include <chrono>
#include <cstdlib>
#include <memory>

#include <efield_estimation/efield_estimation.h>

using namespace std::chrono_literals;


void add(const std::shared_ptr<neuronavigation_interfaces::srv::Efield::Request> request,
          std::shared_ptr<neuronavigation_interfaces::srv::Efield::Response> response)
{
  //Change to efield vector
  std::vector<double> position;
  std::vector<double> orientation;
  position.push_back(request->coordinate.position.x);
  position.push_back(request->coordinate.position.y);
  position.push_back(request->coordinate.position.z);
  orientation.push_back(request->coordinate.orientation.alpha);
  orientation.push_back(request->coordinate.orientation.beta);
  orientation.push_back(request->coordinate.orientation.gamma);

  std::vector<double> efield_vector;
  efield_vector = efield_estimation(position, orientation);
  for (int i = 0; i < efield_vector.size(); i++)
  {
      response->efield_data.push_back(efield_vector[i]);
  }
  //
  RCLCPP_INFO(rclcpp::get_logger("efield"), "Incoming request\nx: %f" " a: %f",
                request->coordinate.position.x, request->coordinate.orientation.alpha);
  RCLCPP_INFO(rclcpp::get_logger("efield"), "sending back response");
}
int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);

  init_efield();

  std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("efield");

  rclcpp::Service<neuronavigation_interfaces::srv::Efield>::SharedPtr service =
    node->create_service<neuronavigation_interfaces::srv::Efield>("efield", &add);
  RCLCPP_INFO(rclcpp::get_logger("efield"), "Efield server ready.");

  rclcpp::spin(node);
  rclcpp::shutdown();
}
