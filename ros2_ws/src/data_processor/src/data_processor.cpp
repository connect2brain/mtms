#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"


#include "headers/scheduling_utils.h"

#include "headers/data_processor.h"

using namespace std::chrono_literals;
using namespace std::chrono;

DataProcessor::DataProcessor() : Node("data_processor") {
  std::string processor_type;
  this->declare_parameter<std::string>("processor_type", "python");
  this->get_parameter("processor_type", processor_type);

  std::string processor_script_path;
  this->declare_parameter<std::string>("processor_script", "processors.python.python_processor");
  this->get_parameter("processor_script", processor_script_path);

  int loop_count;
  this->declare_parameter<int>("loop_count", 5);
  this->get_parameter("loop_count", loop_count);

  std::string filename;
  this->declare_parameter<std::string>("file", "data.txt");
  this->get_parameter("file", filename);

  auto data_subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
  };

  auto trigger_subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::Trigger> message) -> void {
    if (message->index == 1) {
      trigger_out_client->async_send_request(req);
    } else if (message->index == 2) {
      RCLCPP_INFO(this->get_logger(), "Difference between triggers: %lu", message->time_us);
      f << std::to_string(message->time_us) << "\n";
    } else {
      RCLCPP_WARN(rclcpp::get_logger("data_processor"), "Unknown message index %d", message->index);
    }
  };
  eeg_data_subscription = this->create_subscription<mtms_interfaces::msg::EegDatapoint>("/eeg/raw_data",
                                                                                        10,
                                                                                        data_subscription_callback);

  trigger_subscription = this->create_subscription<mtms_interfaces::msg::Trigger>("/eeg/trigger_received",
                                                                                  10,
                                                                                  trigger_subscription_callback);

  trigger_out_client = this->create_client<fpga_interfaces::srv::SendTriggerOutEvent>("/fpga/send_trigger_out_event");

  req = std::make_shared<fpga_interfaces::srv::SendTriggerOutEvent::Request>();

  f.open(filename, std::ios::out | std::ios::trunc);
  
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("data_processor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_SCHEDULING_PRIORITY);
#endif
  auto node = std::make_shared<DataProcessor>();


  rclcpp::spin(node);
  //node->shutdown();
  rclcpp::shutdown();
  return 0;
}
