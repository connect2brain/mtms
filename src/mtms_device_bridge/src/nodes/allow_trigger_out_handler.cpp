#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

class AllowTriggerOut : public rclcpp::Node {
public:
  AllowTriggerOut() : Node("allow_trigger_out") {
    auto qos_persist_latest = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

    allow_trigger_out_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
        "/mtms/trigger_out/allowed",
        qos_persist_latest,
        std::bind(&AllowTriggerOut::allow_trigger_out_callback, this, std::placeholders::_1));
  }

  void ensure_fpga_and_apply_cached_state() {
    if (!is_fpga_ok()) {
      init_fpga();
      return;
    }

    if (has_allow_trigger_out_) {
      apply_allow_trigger_out(last_allow_trigger_out_);
    }
  }

private:
  void apply_allow_trigger_out(bool allow_trigger_out) {
    if (!is_fpga_ok()) {
      RCLCPP_WARN(this->get_logger(), "FPGA not in OK state while handling allow trigger out topic");
      return;
    }

    NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, NiFpga_mTMS_ControlBool_Allowtriggerout, allow_trigger_out));
    RCLCPP_INFO(this->get_logger(), "%s trigger out.", allow_trigger_out ? "Allowing" : "Disallowing");
  }

  void allow_trigger_out_callback(const std_msgs::msg::Bool::SharedPtr msg) {
    last_allow_trigger_out_ = msg->data;
    has_allow_trigger_out_ = true;
    apply_allow_trigger_out(last_allow_trigger_out_);
  }

  bool has_allow_trigger_out_ = false;
  bool last_allow_trigger_out_ = false;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr allow_trigger_out_subscription_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<AllowTriggerOut>();
  RCLCPP_INFO(rclcpp::get_logger("allow_trigger_out_handler"), "Allow trigger out handler ready.");

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
