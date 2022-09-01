#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "headers/processor.h"
#include "headers/python_processor.h"

using namespace std::chrono_literals;


class DataProcessor : public rclcpp::Node {
public:
  DataProcessor() : Node("data_processor") {
    std::string processor_type;
    this->declare_parameter<std::string>("processor_type", "python");
    this->get_parameter("processor_type", processor_type);

    std::string processor_script_path;
    this->declare_parameter<std::string>("processor_script", "");
    this->get_parameter("processor_script", processor_script_path);


    if (processor_type == "python") {
      processor = new PythonProcessor(processor_script_path);
    }

    auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
      processor->data_received(13);
    };

    eeg_data_subscription = this->create_subscription<mtms_interfaces::msg::EegDatapoint>("/eeg/raw_data",
                                                                                          10,
                                                                                          subscription_callback);

    processor->init();
    processor->data_received(13);
  }

  int shutdown() {
    if (processor) {
      return processor->close();
    }
    return 0;
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<mtms_interfaces::msg::EegDatapoint>::SharedPtr eeg_data_subscription;
  ProcessorWrapper *processor;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<DataProcessor>();
  rclcpp::spin(node);
  node->shutdown();
  rclcpp::shutdown();
  return 0;
}
