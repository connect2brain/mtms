#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"


#include "headers/scheduling_utils.h"

#include "headers/eeg_processor.h"
#include "headers/memory_utils.h"

using namespace std::chrono_literals;
using namespace std::chrono;

EegProcessor::EegProcessor() : Node("eeg_processor") {
  std::string processor_type;
  this->declare_parameter<std::string>("processor_type", "compiledmatlab");
  this->get_parameter("processor_type", processor_type);

  std::string processor_script_path;
  this->declare_parameter<std::string>("processor_script", "/home/alqio/workspace/mtms/hotswappable_processors/cppmatlab/compiler/libprocessor_factory.so");
  this->get_parameter("processor_script", processor_script_path);

  int loop_count;
  this->declare_parameter<int>("loop_count", 5);
  this->get_parameter("loop_count", loop_count);

  std::string output_file_name;
  this->declare_parameter<std::string>("file", "output.data");
  this->get_parameter("file", output_file_name);

  should_publish_events = true;

  RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "processor type: %s", processor_type.c_str());

  if (processor_type == "python") {
#ifdef PYTHON_FOUND
    processor = new PythonProcessor(processor_script_path);
#else
    std::cerr << "ERROR: Trying to use python processor but compiled without python" << std::endl;
#endif

  } else if (processor_type == "matlab") {
#ifdef MATLAB_FOUND
    processor = new MatlabProcessor(processor_script_path);
#else
    std::cerr << "ERROR: Trying to use MATLAB processor but compiled without MATLAB" << std::endl;
#endif

  } else if (processor_type == "compiledmatlab") {
    processor = new CompiledMatlabProcessor(processor_script_path);
  }

  auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
    auto start = steady_clock::now();

    auto fpga_events = processor->data_received(*message);

    auto stop = steady_clock::now();

    send_fpga_events(fpga_events);

    if (should_publish_events) {
      publish_events(fpga_events);
    }

    auto total = duration_cast<microseconds>(stop - start);
    if (!fpga_events.empty()) {
      RCLCPP_INFO(this->get_logger(), "Processing took: %lu us", total.count());
      f << std::to_string(total.count()) << "\n";
      f.flush();
    }

  };

  auto start_experiment_callback = [this](
      const std::shared_ptr<fpga_interfaces::srv::StartExperiment::Request> request,
      std::shared_ptr<fpga_interfaces::srv::StartExperiment::Response> response) -> void {

    start_experiment_client->async_send_request(start_experiment_request);
    auto fpga_events = processor->init();
    send_fpga_events(fpga_events);

    response->success = true;
  };

  auto stop_experiment_callback = [this](
      const std::shared_ptr<fpga_interfaces::srv::StopExperiment::Request> request,
      std::shared_ptr<fpga_interfaces::srv::StopExperiment::Response> response) -> void {

    processor->close();
    stop_experiment_client->async_send_request(stop_experiment_request);

    response->success = true;
  };

  eeg_data_subscription = this->create_subscription<mtms_interfaces::msg::EegDatapoint>(
      "/eeg/raw_data",
      10,
      subscription_callback
  );

  start_experiment_service = this->create_service<fpga_interfaces::srv::StartExperiment>(
      "/experiment/start_experiment",
      start_experiment_callback
  );

  stop_experiment_service = this->create_service<fpga_interfaces::srv::StopExperiment>(
      "/experiment/stop_experiment",
      stop_experiment_callback
  );

  start_experiment_client = this->create_client<fpga_interfaces::srv::StartExperiment>("/fpga/start_experiment");
  start_experiment_request = std::make_shared<fpga_interfaces::srv::StartExperiment::Request>();

  stop_experiment_client = this->create_client<fpga_interfaces::srv::StopExperiment>("/fpga/stop_experiment");
  stop_experiment_request = std::make_shared<fpga_interfaces::srv::StopExperiment::Request>();


  pulse_client = this->create_client<fpga_interfaces::srv::SendPulse>("/fpga/send_pulse");
  pulse_request = std::make_shared<fpga_interfaces::srv::SendPulse::Request>();

  charge_client = this->create_client<fpga_interfaces::srv::SendCharge>("/fpga/send_charge");
  charge_request = std::make_shared<fpga_interfaces::srv::SendCharge::Request>();

  discharge_client = this->create_client<fpga_interfaces::srv::SendDischarge>("/fpga/send_discharge");
  discharge_request = std::make_shared<fpga_interfaces::srv::SendDischarge::Request>();

  if (should_publish_events) {
    event_publisher = this->create_publisher<mtms_interfaces::msg::Event>("/mtms/events", 10);
  }

  f.open(output_file_name, std::ios::out | std::ios::trunc);

  processor->init();

  if (loop_count > 0) {
    measure(loop_count);
  }

}

void EegProcessor::publish_events(const std::vector<FpgaEvent> &events) {
  for (FpgaEvent event: events) {
    mtms_interfaces::msg::Event ros_event;

    ros_event.event_type = event.event_type;

    switch (event.event_type) {
      case PULSE:
        ros_event.time_us = event.pulse.event.time_us;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published fpga pulse event timed at %lu.", ros_event.time_us);
        break;

      case CHARGE:
        ros_event.time_us = event.charge.event.time_us;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published charge event timed at %lu.", ros_event.time_us);
        break;

      case DISCHARGE:
        ros_event.time_us = event.discharge.event.time_us;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published discharge event timed at %lu.", ros_event.time_us);
        break;

      case SIGNAL_OUT:
        ros_event.time_us = event.signal_out.event.time_us;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published signal out event timed at %lu.", ros_event.time_us);
        break;

      default:
        RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Warning, unknown fpga event type: %d", event.event_type);
    }

    event_publisher->publish(ros_event);

  }
}

void EegProcessor::send_fpga_events(const std::vector<FpgaEvent> &events) {
  for (const auto &event: events) {
    switch (event.event_type) {
      case PULSE:
        pulse_request->pulse = event.pulse;
        pulse_client->async_send_request(pulse_request);
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Sent fpga pulse event.");
        break;

      case CHARGE:
        charge_request->charge = event.charge;
        charge_client->async_send_request(charge_request);
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Sent fpga charge event.");
        break;

      case DISCHARGE:
        discharge_request->discharge = event.discharge;
        discharge_client->async_send_request(discharge_request);
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Sent fpga discharge event.");
        break;

      default:
        RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Warning, unknown fpga event type: %d", event.event_type);
    }
  }
}

void EegProcessor::measure(int repeats) {
  std::vector<mtms_interfaces::msg::EegDatapoint> events;
  for (auto j = 0; j < repeats; j++) {
    mtms_interfaces::msg::EegDatapoint message = mtms_interfaces::msg::EegDatapoint();
    for (auto i = 0; i < 62; i++) {
      message.eeg_channels.push_back(fRand(0, 100));
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

int EegProcessor::shutdown() {
  RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Shutting down data processor");
  if (processor) {
    processor->close();
    return 0;
  }
  if (f) {
    f.close();
  }
  return 0;
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegProcessor>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  node->shutdown();
  rclcpp::shutdown();
  return 0;
}
