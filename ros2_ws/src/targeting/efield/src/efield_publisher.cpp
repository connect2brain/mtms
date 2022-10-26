#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "neuronavigation_interfaces/msg/pose_using_euler_angles.hpp"

#include "neuronavigation_interfaces/srv/efield.hpp"

#include <memory>

void add(const std::shared_ptr<neuronavigation_interfaces::srv::Efield::Request> request,
          std::shared_ptr<neuronavigation_interfaces::srv::Efield::Response> response)
{
  //Change to efield vector
  response->efield_data.push_back(request->coordinate.position.x);
  response->efield_data.push_back(request->coordinate.position.y);
  response->efield_data.push_back(request->coordinate.position.z);
  response->efield_data.push_back(request->coordinate.orientation.alpha);
  response->efield_data.push_back(request->coordinate.orientation.beta);
  response->efield_data.push_back(request->coordinate.orientation.gamma);
  //
  RCLCPP_INFO(rclcpp::get_logger("efield"), "Incoming request\nx: %f" " a: %f",
                request->coordinate.position.x, request->coordinate.orientation.alpha);
  RCLCPP_INFO(rclcpp::get_logger("efield"), "sending back response");
}

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);

  std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("efield");

  rclcpp::Service<neuronavigation_interfaces::srv::Efield>::SharedPtr service =
    node->create_service<neuronavigation_interfaces::srv::Efield>("efield", &add);

  RCLCPP_INFO(rclcpp::get_logger("efield"), "Efield server ready.");

  rclcpp::spin(node);
  rclcpp::shutdown();
}