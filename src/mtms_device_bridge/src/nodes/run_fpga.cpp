#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"
#include "system_interfaces/msg/healthcheck.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"

#include <sstream>

const std::string HEALTHCHECK_TOPIC = "/mtms/device/healthcheck";

class FpgaConnection : public rclcpp::Node {
public:
  FpgaConnection(): Node("fpga_connection") {
    this->healthcheck_publisher = this->create_publisher<system_interfaces::msg::Healthcheck>(HEALTHCHECK_TOPIC, 10);
  }

  void publish_healthcheck(uint8_t status_value, const std::string &status_message, const std::string &actionable_message) {
    auto healthcheck = system_interfaces::msg::Healthcheck();

    healthcheck.status = status_value;
    healthcheck.status_message = status_message;
    healthcheck.actionable_message = actionable_message;

    this->healthcheck_publisher->publish(healthcheck);
  }

private:
  rclcpp::Publisher<system_interfaces::msg::Healthcheck>::SharedPtr healthcheck_publisher;
};

/* Initialize FPGA with health checks */
void init_fpga_with_healthcheck(std::shared_ptr<FpgaConnection> node, bool first_time) {
  int waiting_time_left = first_time ? 0 : 60;

  uint8_t status_value;
  while (rclcpp::ok()) {
    if (init_fpga()) {
      RCLCPP_INFO(node->get_logger(), "FPGA initialized successfully.");
      break;
    }

    if (waiting_time_left > 0) {
      std::ostringstream oss;
      oss << "Please wait for " << waiting_time_left << " seconds before powering on the mTMS device.";
      std::string msg = oss.str();

      status_value = system_interfaces::msg::Healthcheck::NOT_READY;
      node->publish_healthcheck(status_value, "mTMS device not powered on", msg);

      waiting_time_left--;
    } else {
      status_value = system_interfaces::msg::Healthcheck::NOT_READY;
      node->publish_healthcheck(status_value, "mTMS device not powered on", "Please power on the mTMS device.");
    }

    rclcpp::spin_some(node);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (!rclcpp::ok()) {
    RCLCPP_WARN(node->get_logger(), "Shutdown signal received during FPGA initialization.");
  }
}

/* Run the FPGA operation non-blocking */
void run_fpga(std::shared_ptr<FpgaConnection> node) {
  NiFpga_Status ni_status = NiFpga_Run(session, 0); // Non-blocking mode
  NiFpga_MergeStatus(&status, ni_status);

  if (NiFpga_IsError(status)) {
    RCLCPP_ERROR(node->get_logger(), "NiFpga_Run failed with status: %u", status);
    node->publish_healthcheck(
      system_interfaces::msg::Healthcheck::ERROR,
      "FPGA Run Failed",
      "Failed to start FPGA operation."
    );
    return;
  }

  uint32_t state = NiFpga_FpgaViState_NotRunning;
  while (rclcpp::ok()) {
    ni_status = NiFpga_GetFpgaViState(session, &state);

    if (NiFpga_IsError(ni_status)) {
      RCLCPP_ERROR(node->get_logger(), "NiFpga_GetFpgaViState failed with status: %u", ni_status);
      node->publish_healthcheck(
        system_interfaces::msg::Healthcheck::ERROR,
        "FPGA State Check Failed",
        "Failed to retrieve FPGA state."
      );
      break;
    }

    if (state == NiFpga_FpgaViState_NaturallyStopped) {
      RCLCPP_INFO(node->get_logger(), "FPGA operation completed successfully.");
      break;
    }

    if (state == NiFpga_FpgaViState_NotRunning) {
      RCLCPP_WARN(node->get_logger(), "FPGA is not running.");
      break;
    }

    rclcpp::spin_some(node);

    // Check status every 100ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (!rclcpp::ok()) {
    RCLCPP_INFO(node->get_logger(), "Shutdown signal received. Aborting FPGA run.");
    NiFpga_Abort(session);
  }
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<FpgaConnection>();
  bool first_time = true;

  while (rclcpp::ok()) {
    init_fpga_with_healthcheck(node, first_time);
    first_time = false;

    run_fpga(node);
    close_fpga();
  }

  close_fpga();
  rclcpp::shutdown();
  RCLCPP_INFO(node->get_logger(), "Shutdown complete.");
  return 0;
}
