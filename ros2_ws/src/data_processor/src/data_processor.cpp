#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"


#include "headers/scheduling_utils.h"

#include "headers/data_processor.h"
#include "headers/memory_utils.h"

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

  std::string output_file_name;
  this->declare_parameter<std::string>("file", "output.data");
  this->get_parameter("file", output_file_name);

  RCLCPP_INFO(rclcpp::get_logger("data_processor"), "processor type: %s", processor_type.c_str());

  if (processor_type == "python") {
#ifdef PYTHON_FOUND
    processor = new PythonProcessor(processor_script_path);
#else
    std::err << "ERROR: Trying to use python processor but compiled without python" << std::endl;
#endif

  } else if (processor_type == "matlab") {
#ifdef MATLAB_FOUND
    processor = new MatlabProcessor(processor_script_path);
#else
    std::err << "ERROR: Trying to use MATLAB processor but compiled without MATLAB" << std::endl;
#endif

  } else if (processor_type == "compiledmatlab") {
    processor = new CompiledMatlabProcessor(processor_script_path);
  }

  auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
    auto start = steady_clock::now();

    auto fpga_events = processor->data_received(*message);

    auto stop = steady_clock::now();
    auto total = duration_cast<microseconds>(stop - start);
    //RCLCPP_INFO(this->get_logger(), "Duration: %lu us", total.count());
    f << std::to_string(total.count()) << "\n";

  };

  eeg_data_subscription = this->create_subscription<mtms_interfaces::msg::EegDatapoint>("/eeg/raw_data",
                                                                                        10,
                                                                                        subscription_callback);

  f.open(output_file_name, std::ios::out | std::ios::trunc);

  processor->init();

  if (loop_count > 0) {
    measure(loop_count);
  }

}

void DataProcessor::measure(int repeats) {
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
    auto start = steady_clock::now();

    auto fpga_events = processor->data_received(events[i]);

    auto stop = steady_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
    times.push_back(duration);
    total = duration_cast<microseconds>(duration + total);

    for (auto event: fpga_events) {
      event.print();
      std::cout << "---" << std::endl;
    }

  }
  RCLCPP_INFO(this->get_logger(), "Duration total: %lu us", total.count());
  RCLCPP_INFO(this->get_logger(), "Average execution time: %f us", ((double) total.count()) / repeats);
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("data_processor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<DataProcessor>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("data_processor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  node->shutdown();
  rclcpp::shutdown();
  return 0;
}
