#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"

#include "headers/processor.h"
#include "headers/python_processor.h"
#include "headers/matlab_processor.h"
#include "headers/cpp_processor.h"

#include "headers/scheduling_utils.h"

using namespace std::chrono_literals;
using namespace std::chrono;

double fRand(double fMin, double fMax) {
  double f = (double) rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

class DataProcessor : public rclcpp::Node {
public:
  DataProcessor() : Node("data_processor") {
    RCLCPP_INFO(this->get_logger(), "in data_processor constructor");
    std::string processor_type;
    this->declare_parameter<std::string>("processor_type", "python");
    this->get_parameter("processor_type", processor_type);

    std::string processor_script_path;
    this->declare_parameter<std::string>("processor_script", "");
    this->get_parameter("processor_script", processor_script_path);

    int loop_count;
    this->declare_parameter<int>("loop_count", 10);
    this->get_parameter("loop_count", loop_count);

    RCLCPP_INFO(rclcpp::get_logger("data_processor"), "processor type: %s", processor_type.c_str());
    if (processor_type == "python") {
      processor = new PythonProcessor(processor_script_path);
    } else if (processor_type == "matlab") {
      processor = new MatlabProcessor(processor_script_path);
    } else if (processor_type == "cpp") {
      processor = new CPPProcessor(processor_script_path);
    }

    auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
      auto start = high_resolution_clock::now();

      auto fpga_events = processor->data_received(*message);

      auto stop = high_resolution_clock::now();
      auto total = duration_cast<microseconds>(stop - start);
      RCLCPP_INFO(this->get_logger(), "Duration: %lu us", total.count());

    };

    mtms_interfaces::msg::EegDatapoint message = mtms_interfaces::msg::EegDatapoint();
    for (auto i = 0; i < 62; i++) {
      message.channel_datapoint.push_back(fRand(0, 100));
    }
    processor->init();

    eeg_data_subscription = this->create_subscription<mtms_interfaces::msg::EegDatapoint>("/eeg/raw_data",
                                                                                          10,
                                                                                          subscription_callback);
    //measure(loop_count);
  }

  void measure(int repeats) {
    std::vector<mtms_interfaces::msg::EegDatapoint> events;
    for (auto j = 0; j < repeats; j++) {
      mtms_interfaces::msg::EegDatapoint message = mtms_interfaces::msg::EegDatapoint();
      for (auto i = 0; i < 62; i++) {
        message.channel_datapoint.push_back(fRand(0, 100));
      }
      events.push_back(message);
    }
    std::vector<std::chrono::microseconds> times;
    std::chrono::microseconds total = std::chrono::microseconds(0s);
    for (auto i = 0; i < repeats; i++) {
      auto start = high_resolution_clock::now();

      auto fpga_events = processor->data_received(events[i]);
      auto stop = high_resolution_clock::now();
      auto duration = duration_cast<microseconds>(stop - start);
      times.push_back(duration);
      total = duration_cast<microseconds>(duration + total);
    }
    RCLCPP_INFO(this->get_logger(), "Duration total: %lu us", total.count());
    RCLCPP_INFO(this->get_logger(), "Average execution time: %f us", ((double) total.count()) / repeats);
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

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_SCHEDULING_PRIORITY);
#endif

  rclcpp::spin(node);
  node->shutdown();
  rclcpp::shutdown();
  return 0;
}
