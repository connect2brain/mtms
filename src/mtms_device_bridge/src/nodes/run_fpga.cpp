#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/string.hpp"
#include "mtms_system_interfaces/msg/component_health.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"

#include <sstream>

const std::string HEALTH_TOPIC = "/mtms/device/health";

class FpgaConnection : public rclcpp::Node {
public:
  FpgaConnection(): Node("fpga_connection") {
    this->health_publisher = this->create_publisher<mtms_system_interfaces::msg::ComponentHealth>(HEALTH_TOPIC, 10);
  }

  void publish_health(uint8_t health_level, const std::string &message) {
    auto health = mtms_system_interfaces::msg::ComponentHealth();

    health.health_level = health_level;
    health.message = message;

    this->health_publisher->publish(health);
  }

private:
  rclcpp::Publisher<mtms_system_interfaces::msg::ComponentHealth>::SharedPtr health_publisher;
};

/* Initialize FPGA with health checks */
void init_fpga_with_health(std::shared_ptr<FpgaConnection> node, bool first_time) {
  int waiting_time_left = first_time ? 0 : 60;

  while (rclcpp::ok()) {
    if (init_fpga()) {
      RCLCPP_INFO(node->get_logger(), "FPGA initialized successfully.");
      break;
    }

    if (waiting_time_left > 0) {
      std::ostringstream oss;
      oss << "Please wait for " << waiting_time_left << " seconds before powering on the mTMS device.";
      std::string msg = oss.str();

      node->publish_health(mtms_system_interfaces::msg::ComponentHealth::DEGRADED, msg);

      waiting_time_left--;
    } else {
      node->publish_health(mtms_system_interfaces::msg::ComponentHealth::DEGRADED, "Please power on the mTMS device.");
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
    node->publish_health(
      mtms_system_interfaces::msg::ComponentHealth::ERROR,
      "FPGA run failed: failed to start FPGA operation."
    );
    return;
  }

  uint32_t state = NiFpga_FpgaViState_NotRunning;
  while (rclcpp::ok()) {
    ni_status = NiFpga_GetFpgaViState(session, &state);

    if (NiFpga_IsError(ni_status)) {
      RCLCPP_ERROR(node->get_logger(), "NiFpga_GetFpgaViState failed with status: %u", ni_status);
      node->publish_health(
        mtms_system_interfaces::msg::ComponentHealth::ERROR,
        "FPGA state check failed: failed to retrieve FPGA state."
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
    init_fpga_with_health(node, first_time);
    first_time = false;

    run_fpga(node);
    close_fpga();
  }

  close_fpga();
  rclcpp::shutdown();
  RCLCPP_INFO(node->get_logger(), "Shutdown complete.");
  return 0;
}
