#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"

const std::string NODE_MESSAGE_TOPIC = "/node/message";

class FpgaConnection : public rclcpp::Node {
public:
  FpgaConnection(): Node("fpga_connection") {
    this->publisher_node_message_ = this->create_publisher<std_msgs::msg::String>(NODE_MESSAGE_TOPIC, 10);
  }
  void send_node_message(std::string str) {
    auto msg = std_msgs::msg::String();
    msg.data = str;
    this->publisher_node_message_->publish(msg);
  }

private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_node_message_;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<FpgaConnection>();

  bool fpga_initialized = false;
  while (rclcpp::ok() && !fpga_initialized) {
    if (!fpga_initialized) {

      fpga_initialized = try_init_fpga();
      if (!fpga_initialized) {
        node->send_node_message("Please power on the mTMS device.");

        RCLCPP_WARN(rclcpp::get_logger("fpga_connection"), "Initialization attempt failed. Retrying...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
    rclcpp::spin_some(node);
  }

  RCLCPP_INFO(rclcpp::get_logger("fpga_connection"), "Initialization successful, running FPGA.");

  rclcpp::shutdown();

  /* TODO: Seems to not terminate gracefully when receiving SIGINT, causing a benign error in the logs. */
  NiFpga_MergeStatus(&status, NiFpga_Run(session, NiFpga_RunAttribute_WaitUntilDone));

  close_fpga();
}
