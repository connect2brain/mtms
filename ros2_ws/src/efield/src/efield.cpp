#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "neuronavigation_interfaces/msg/pose_using_euler_angles.hpp"
#include "shape_msgs/msg/mesh.hpp"

using std::placeholders::_1;

class MinimalSubscriber : public rclcpp::Node
{
  public:
    MinimalSubscriber() : Node("efield")
    {
      subscription_pose_ = this->create_subscription<neuronavigation_interfaces::msg::PoseUsingEulerAngles>(
      "neuronavigation/coil_pose", 10, std::bind(&MinimalSubscriber::topic_callback_pose, this, _1));
       subscription_mesh_ = this->create_subscription<shape_msgs::msg::Mesh>(
      "neuronavigation/coil_mesh", 10, std::bind(&MinimalSubscriber::topic_callback_mesh, this, _1));
    }

  private:
    void topic_callback_pose(const neuronavigation_interfaces::msg::PoseUsingEulerAngles::SharedPtr msg) const
    {
      RCLCPP_INFO(this->get_logger(), "X: %f Y: %f Z: %f", msg->position.x, msg->position.y, msg->position.z);
    }
    rclcpp::Subscription<neuronavigation_interfaces::msg::PoseUsingEulerAngles>::SharedPtr subscription_pose_;

    void topic_callback_mesh(const shape_msgs::msg::Mesh::SharedPtr msg) const
    {
      RCLCPP_INFO(this->get_logger(), "vertices: %f triangles: %f", msg->vertices[0].x, msg->triangles);
    }
    rclcpp::Subscription<shape_msgs::msg::Mesh>::SharedPtr subscription_mesh_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}
