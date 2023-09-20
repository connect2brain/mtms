#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_device_interfaces/srv/allow_stimulation.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using std::placeholders::_1;

/* HACK: Needs to match the value in neuronavigation bridge. */
const milliseconds COIL_AT_TARGET_PUBLISHING_INTERVAL = 100ms;

class StimulationAllower : public rclcpp::Node {

public:
  StimulationAllower() : Node("stimulation_allower") {

    /* Create QoS profile for 'coil at target' subscriber. */
    const uint64_t deadline_ns = static_cast<uint64_t>(std::chrono::nanoseconds(COIL_AT_TARGET_PUBLISHING_INTERVAL).count());
    const rmw_time_t rmw_deadline = {0, deadline_ns};
    const rmw_time_t rmw_lifespan = rmw_deadline;

    const rmw_qos_profile_t qos_profile = {
        RMW_QOS_POLICY_HISTORY_KEEP_LAST,
        1,
        RMW_QOS_POLICY_RELIABILITY_RELIABLE,
        RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
        rmw_deadline,
        rmw_lifespan,
        RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT,
        RMW_QOS_LIVELINESS_LEASE_DURATION_DEFAULT,
        false
    };
    auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, qos_profile.depth), qos_profile);

    rclcpp::SubscriptionOptions subscription_options;
    subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) -> void {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "'Coil at target' message not received within deadline, disallowing stimulation.");

      this->allow_stimulation(false);
    };

    /* Create subscriber and service client. */
    subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "/neuronavigation/coil_at_target", qos, std::bind(&StimulationAllower::coil_at_target_callback, this, _1));

    client_ = this->create_client<mtms_device_interfaces::srv::AllowStimulation>("/mtms_device/allow_stimulation");
  }

private:
  void allow_stimulation(bool value) {
    RCLCPP_INFO(this->get_logger(), "%s stimulation.", value ? "Allowing" : "Disallowing");

    auto request = std::make_shared<mtms_device_interfaces::srv::AllowStimulation::Request>();
    request->allow_stimulation = value;

    while (!client_->wait_for_service(std::chrono::seconds(1))) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
        return;
      }
      RCLCPP_INFO(this->get_logger(), "Waiting for service to become available...");
    }
    auto result_future = client_->async_send_request(request);
  }

  void coil_at_target_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    allow_stimulation(msg->data);
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
