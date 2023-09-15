#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_device_interfaces/srv/allow_stimulation.hpp"

using namespace std;
using std::placeholders::_1;

class StimulationAllower : public rclcpp::Node {

public:
  StimulationAllower() : Node("stimulation_allower") {
    /* TODO: Add a deadline for coil_at_target messages; if missed, default to fallback behavior,
         disallowing pulses.
    */
    subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "/neuronavigation/coil_at_target", 10, std::bind(&StimulationAllower::coil_at_target_callback, this, _1));

    client_ = this->create_client<mtms_device_interfaces::srv::AllowStimulation>("/mtms_device/allow_stimulation");
  }

private:
  void coil_at_target_callback(const std_msgs::msg::Bool::SharedPtr msg) {

    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data ? "Yes" : "No");

    auto request = std::make_shared<mtms_device_interfaces::srv::AllowStimulation::Request>();
    request->allow_stimulation = msg->data;

    while (!client_->wait_for_service(std::chrono::seconds(1))) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
        return;
      }
      RCLCPP_INFO(this->get_logger(), "Waiting for service to become available...");
    }
    auto result_future = client_->async_send_request(request);
  }
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscription_;
  rclcpp::Client<mtms_device_interfaces::srv::AllowStimulation>::SharedPtr client_;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("stimulation_allower"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StimulationAllower>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("stimulation_allower"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
