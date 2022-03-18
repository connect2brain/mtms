#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "neuronavigation_interfaces/msg/pose_using_euler_angles.hpp"
#include "shape_msgs/msg/mesh.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
* member function as a callback from the timer. */

class EfieldPublisher : public rclcpp::Node
{
  public:
    EfieldPublisher()
    : Node("efield"), count_(0)
    {
      publisher_ = this->create_publisher<std_msgs::msg::String>("efield", 10);
      timer_ = this->create_wall_timer(
      500ms, std::bind(&EfieldPublisher::timer_callback, this));
    }

  private:
    void timer_callback()
    {
      auto message = std_msgs::msg::String();
      message.data = "Hello, world! " + std::to_string(count_++);
      RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
      publisher_->publish(message);
    }
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    size_t count_;
};


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<EfieldPublisher>());
  rclcpp::shutdown();
  return 0;
}
