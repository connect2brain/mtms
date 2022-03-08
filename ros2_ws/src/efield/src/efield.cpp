#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "neuronavigation_interfaces/msg/pose_using_euler_angles.hpp"

using std::placeholders::_1;

class MinimalSubscriber : public rclcpp::Node
{
  public:
    MinimalSubscriber() : Node("efield")
    {
      subscription_ = this->create_subscription<neuronavigation_interfaces::msg::PoseUsingEulerAngles>(
      "neuronavigation/coil_pose", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));
    }

  private:
    void topic_callback(const neuronavigation_interfaces::msg::PoseUsingEulerAngles::SharedPtr msg) const
    {
      RCLCPP_INFO(this->get_logger(), "X: %f Y: %f Z: %f", msg->position.x, msg->position.y, msg->position.z);
    }
    rclcpp::Subscription<neuronavigation_interfaces::msg::PoseUsingEulerAngles>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}
