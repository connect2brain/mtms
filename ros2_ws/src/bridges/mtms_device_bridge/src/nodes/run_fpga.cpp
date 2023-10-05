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

/* Otherwise similar to the shared init_fpga() function, used by the other ROS nodes in the
   mTMS device bridge, except that:

     - Sends status messages to the UI
     - Once the initialization is finished, runs the FPGA program (one and only one of the ROS nodes
       needs to do that - the others can then use the already running program.)*/
void init_and_run_fpga(std::shared_ptr<FpgaConnection> node) {
  while (true) {
    if (try_init_fpga()) {
      break;
    }
    node->send_node_message("Please power on the mTMS device.");
    rclcpp::spin_some(node);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  NiFpga_MergeStatus(&status, NiFpga_Run(session, NiFpga_RunAttribute_WaitUntilDone));
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<FpgaConnection>();

  init_and_run_fpga(node);

  while (rclcpp::ok()) {
    if (!is_fpga_ok()) {
      close_fpga();
      init_and_run_fpga(node);
    }
    rclcpp::spin_some(node);
  }
  close_fpga();
  rclcpp::shutdown();
}
