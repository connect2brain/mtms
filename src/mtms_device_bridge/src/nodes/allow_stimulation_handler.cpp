#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/empty.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

class AllowStimulation : public rclcpp::Node {
public:
  AllowStimulation() : Node("allow_stimulation") {
    auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

    allow_stimulation_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
        "/mtms/stimulation/allowed",
        qos_persist_latest,
        std::bind(&AllowStimulation::allow_stimulation_callback, this, std::placeholders::_1));

    const std::string heartbeat_topic = "/mtms/device/heartbeat";
    auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(heartbeat_topic, 10);
    this->create_wall_timer(std::chrono::milliseconds(500), [heartbeat_publisher]() {
      heartbeat_publisher->publish(std_msgs::msg::Empty());
    });
  }

  void ensure_fpga_and_apply_cached_state() {
    if (!is_fpga_ok()) {
      init_fpga();
      return;
    }

    if (has_allow_stimulation_) {
      apply_allow_stimulation(last_allow_stimulation_);
    }
  }

private:
  void apply_allow_stimulation(bool allow_stimulation) {
    if (!is_fpga_ok()) {
      RCLCPP_WARN(this->get_logger(), "FPGA not in OK state while handling allow stimulation topic");
      return;
    }

    NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Allowstimulation, allow_stimulation));
    RCLCPP_INFO(this->get_logger(), "%s stimulation", allow_stimulation ? "Allowing" : "Disallowing");
  }

  void allow_stimulation_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    last_allow_stimulation_ = msg->data;
    has_allow_stimulation_ = true;
    apply_allow_stimulation(last_allow_stimulation_);
  }

  bool has_allow_stimulation_ = false;
  bool last_allow_stimulation_ = false;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr allow_stimulation_subscription_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<AllowStimulation>();
  RCLCPP_INFO(rclcpp::get_logger("allow_stimulation"), "Allow stimulation handler ready.");

  auto timer = node->create_wall_timer(
      std::chrono::milliseconds(FPGA_OK_CHECK_INTERVAL_MS),
      [&]() {
          node->ensure_fpga_and_apply_cached_state();
      }
  );
  rclcpp::spin(node);

  close_fpga();
  rclcpp::shutdown();
}
