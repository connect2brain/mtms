#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_device_interfaces/srv/allow_stimulation.hpp"
#include "stimulation_interfaces/srv/is_stimulation_allowed.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using std::placeholders::_1;

/* HACK: Needs to match the value in neuronavigation bridge. */
const milliseconds COIL_AT_TARGET_PUBLISHING_INTERVAL = 600ms;

class StimulationAllower : public rclcpp::Node {

public:
  StimulationAllower() : Node("stimulation_allower") {

    auto is_stimulation_allowed_callback = [this](
        [[maybe_unused]] const std::shared_ptr<stimulation_interfaces::srv::IsStimulationAllowed::Request> request,
        std::shared_ptr<stimulation_interfaces::srv::IsStimulationAllowed::Response> response) -> void {

      response->success = true;
      response->stimulation_allowed = this->stimulation_allowed;

      RCLCPP_INFO(rclcpp::get_logger("stimulation_allower"), "Successfully responded to get stimulation allowed request.");
    };

    /* Create QoS profile for 'coil at target' subscriber. */
    auto qos = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
        .deadline(std::chrono::nanoseconds(COIL_AT_TARGET_PUBLISHING_INTERVAL))
        .liveliness(RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT);

    rclcpp::SubscriptionOptions subscription_options;
    subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) -> void {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "'Coil at target' message was not received within deadline, setting to false.");

      this->coil_at_target = false;
      this->update_stimulation_allowed();
    };

    /* Create subscribers and service client. */
    target_mode_enabled_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "/neuronavigation/target_mode/enabled", 10, std::bind(&StimulationAllower::target_mode_enabled_callback, this, _1));

    coil_at_target_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "/neuronavigation/coil_at_target", qos, std::bind(&StimulationAllower::coil_at_target_callback, this, _1), subscription_options);

    client_ = this->create_client<mtms_device_interfaces::srv::AllowStimulation>("/mtms_device/allow_stimulation");

    /* Create service for querying status of stimulation allowed.*/
    is_stimulation_allowed_service_ = this->create_service<stimulation_interfaces::srv::IsStimulationAllowed>(
        "/stimulation/allowed", is_stimulation_allowed_callback);
  }

private:
  void update_stimulation_allowed() {
    /* Allow stimulation if:
          - target mode is enabled and coil is at the target, or
          - target mode is disabled. */
    this->stimulation_allowed = !this->target_mode_enabled || this->coil_at_target;

    RCLCPP_INFO(this->get_logger(), "%s stimulation.", this->stimulation_allowed ? "Allowing" : "Disallowing");

    /* Send the new status to the mTMS device. */
    auto request = std::make_shared<mtms_device_interfaces::srv::AllowStimulation::Request>();
    request->allow_stimulation = this->stimulation_allowed;

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
    this->coil_at_target = msg->data;

    RCLCPP_INFO(this->get_logger(), "Coil is%sat target.", this->coil_at_target ? " " : " not ");

    update_stimulation_allowed();
  }

  void target_mode_enabled_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    this->target_mode_enabled = msg->data;

    RCLCPP_INFO(this->get_logger(), "Target mode %s.", this->target_mode_enabled ? "enabled" : "disabled");

    update_stimulation_allowed();
  }

  bool coil_at_target = false;
  bool target_mode_enabled = false;
  bool stimulation_allowed = true;

  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr target_mode_enabled_subscription_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr coil_at_target_subscription_;
  rclcpp::Client<mtms_device_interfaces::srv::AllowStimulation>::SharedPtr client_;
  rclcpp::Service<stimulation_interfaces::srv::IsStimulationAllowed>::SharedPtr is_stimulation_allowed_service_;
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
