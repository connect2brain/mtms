#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"
#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"

#include <sstream>

const std::string HEALTHCHECK_TOPIC = "/mtms_device/healthcheck";

class FpgaConnection : public rclcpp::Node {
public:
  FpgaConnection(): Node("fpga_connection") {
    this->publisher_healthcheck_ = this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);
  }

  void publish_healthcheck(uint8_t status_value, std::string status_message, std::string actionable_message) {
    auto healthcheck = system_interfaces::msg::Healthcheck();

    healthcheck.status.value = status_value;
    healthcheck.status_message = status_message;
    healthcheck.actionable_message = actionable_message;

    this->publisher_healthcheck_->publish(healthcheck);
  }

private:
  rclcpp::Publisher<system_interfaces::msg::Healthcheck>::SharedPtr publisher_healthcheck_;
};

/* Otherwise similar to the shared init_fpga() function, used by the other ROS nodes in the
   mTMS device bridge, except that:

     - Sends healthcheck messages to the UI
     - If the FPGA has been powered off during the same session, ask the user to wait for one minute
       before powering on again. */
void init_fpga_with_healthcheck(std::shared_ptr<FpgaConnection> node, bool first_time) {
  int waiting_time_left = first_time ? 0 : 60;

  uint8_t status_value;
  while (true) {
    if (try_init_fpga()) {
      break;
    }
    if (waiting_time_left > 0) {
      std::ostringstream oss;
      oss << "Please wait for " << waiting_time_left << " seconds before powering on the mTMS device";
      std::string str = oss.str();

      status_value = system_interfaces::msg::HealthcheckStatus::NOT_READY;
      node->publish_healthcheck(status_value, "mTMS device not powered on", str);

      waiting_time_left--;
    } else {
      status_value = system_interfaces::msg::HealthcheckStatus::NOT_READY;
      node->publish_healthcheck(status_value, "mTMS device not powered on", "Please power on the mTMS device.");
    }
    rclcpp::spin_some(node);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  status_value = system_interfaces::msg::HealthcheckStatus::READY;
  node->publish_healthcheck(status_value, "Ready", "");
}

void run_fpga() {
  NiFpga_MergeStatus(&status, NiFpga_Run(session, NiFpga_RunAttribute_WaitUntilDone));
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<FpgaConnection>();

  bool first_time = true;
  while (rclcpp::ok()) {
    init_fpga_with_healthcheck(node, first_time);
    first_time = false;

    run_fpga();
    close_fpga();
  }

  close_fpga();
  rclcpp::shutdown();
}
