#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/empty.hpp"
#include "mtms_system_interfaces/srv/get_stimulation_allowed.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
using std::placeholders::_1;

/* HACK: Needs to match the value in neuronavigation bridge. */
const milliseconds COIL_AT_TARGET_PUBLISHING_INTERVAL = 600ms;

const std::string ALLOW_STIMULATION_TOPIC_NAME = "/mtms/stimulation/allowed";
const std::string ALLOW_TRIGGER_OUT_TOPIC_NAME = "/mtms/trigger_out/allowed";
const std::string HEARTBEAT_TOPIC = "/mtms/stimulation_allower/heartbeat";
constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

class StimulationAllower : public rclcpp::Node {

public:
  StimulationAllower() : Node("stimulation_allower") {

    auto get_stimulation_allowed_callback = [this](
        [[maybe_unused]] const std::shared_ptr<mtms_system_interfaces::srv::GetStimulationAllowed::Request> request,
        std::shared_ptr<mtms_system_interfaces::srv::GetStimulationAllowed::Response> response) -> void {

      response->success = true;
      response->stimulation_allowed = this->stimulation_allowed;

      RCLCPP_INFO(rclcpp::get_logger("stimulation_allower"), "Successfully responded to get stimulation allowed request.");
    };

    /* Create QoS to persist the latest sample. */
    auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

    /* Create QoS profile for 'coil at target' subscriber. */
    auto qos_deadline = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
        .deadline(std::chrono::nanoseconds(COIL_AT_TARGET_PUBLISHING_INTERVAL));

    rclcpp::SubscriptionOptions subscription_options;
    subscription_options.event_callbacks.deadline_callback = [this]([[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo & event) -> void {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "'Coil at target' message was not received within deadline, setting to false.");

      this->coil_at_target = false;
      this->update_stimulation_allowed();
    };

    /* Create subscribers and publishers. */
    neuronavigation_started_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "/neuronavigation/started", qos_persist_latest, std::bind(&StimulationAllower::neuronavigation_started_callback, this, _1));

    target_mode_enabled_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "/neuronavigation/target_mode/enabled", qos_persist_latest, std::bind(&StimulationAllower::target_mode_enabled_callback, this, _1));

    coil_at_target_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "/neuronavigation/coil_at_target", qos_deadline, std::bind(&StimulationAllower::coil_at_target_callback, this, _1), subscription_options);

    allow_stimulation_publisher_ = this->create_publisher<std_msgs::msg::Bool>(ALLOW_STIMULATION_TOPIC_NAME, qos_persist_latest);
    allow_trigger_out_publisher_ = this->create_publisher<std_msgs::msg::Bool>(ALLOW_TRIGGER_OUT_TOPIC_NAME, qos_persist_latest);

    /* Create service for querying status of stimulation allowed.*/
    get_stimulation_allowed_service_ = this->create_service<mtms_system_interfaces::srv::GetStimulationAllowed>(
        "/mtms/stimulation/get_allowed", get_stimulation_allowed_callback);

    /* Update initial default state even if no neuronavigation topics are publishing. */
    update_stimulation_allowed();

    RCLCPP_INFO(this->get_logger(), "Stimulation allower ready.");

    auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
    this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
      heartbeat_publisher->publish(std_msgs::msg::Empty());
    });
  }

private:
  void publish_stimulation_allowed(bool value) {
    std_msgs::msg::Bool msg;
    msg.data = value;
    allow_stimulation_publisher_->publish(msg);
  }

  void publish_trigger_out_allowed(bool value) {
    std_msgs::msg::Bool msg;
    msg.data = value;
    allow_trigger_out_publisher_->publish(msg);
  }

  void update_stimulation_allowed() {
    RCLCPP_INFO(this->get_logger(), "Neuronavigation: %s, target mode: %s, coil at target: %s",
      this->neuronavigation_started ? "started" : "stopped",
      this->target_mode_enabled ? "enabled" : "disabled",
      this->coil_at_target ? "true" : "false");

    /* Allow stimulation if:
          - neuronavigation is not started,
          - neuronavigation is started but target mode is disabled, or
          - neuronavigation is started, target mode is enabled, and coil is at the target. */
    this->stimulation_allowed = !this->neuronavigation_started || !this->target_mode_enabled || this->coil_at_target;

    RCLCPP_INFO(this->get_logger(), "%s stimulation and trigger out.", this->stimulation_allowed ? "Allowing" : "Disallowing");

    this->publish_stimulation_allowed(this->stimulation_allowed);

    /* If stimulation is disallowed, disallow trigger out, as well.

      Rationale: usually, trigger out is used to either signal that a stimulation pulse has been given or trigger
      stimulation on another TMS device - if stimulation is disallowed, both of these use cases should be
      prevented as well. */
    this->publish_trigger_out_allowed(this->stimulation_allowed);
  }

  void neuronavigation_started_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    this->neuronavigation_started = msg->data;
    update_stimulation_allowed();
  }

  void target_mode_enabled_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    this->target_mode_enabled = msg->data;
    update_stimulation_allowed();
  }

  void coil_at_target_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    this->coil_at_target = msg->data;
    update_stimulation_allowed();
  }

  bool neuronavigation_started = false;
  bool coil_at_target = false;
  bool target_mode_enabled = false;

  bool stimulation_allowed = true;

  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr neuronavigation_started_subscription_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr target_mode_enabled_subscription_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr coil_at_target_subscription_;

  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr allow_stimulation_publisher_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr allow_trigger_out_publisher_;

  rclcpp::Service<mtms_system_interfaces::srv::GetStimulationAllowed>::SharedPtr get_stimulation_allowed_service_;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<StimulationAllower>();

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
