#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"

#include "headers/scheduling_utils.h"
#include "headers/eeg_trigger_processor.h"
#include "headers/memory_utils.h"

using namespace std::chrono_literals;
using namespace std::chrono;

EEGTriggerProcessor::EEGTriggerProcessor() : Node("eeg_trigger_processor") {
  int data_size;
  this->declare_parameter<int>("data_size", 10000);
  this->get_parameter("data_size", data_size);

  std::string filename;
  this->declare_parameter<std::string>("file", "data.txt");
  this->get_parameter("file", filename);

  auto trigger_subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::Trigger> message) -> void {

    auto signal_out = event_interfaces::msg::SignalOut();

    signal_out.port = 2;
    signal_out.duration_us = 10000;
    signal_out.event_info.execution_time = 0.0;
    signal_out.event_info.execution_condition.value = 2;
    signal_out.event_info.id = 1;

    this->signal_out_publisher->publish(signal_out);

    if (message->index == 2) {
      if (index < durations.size()) {
        durations[index++] = message->time;
      } else {
        index++;
      }

      if (index % (durations.size() / 10) == 0) {
        RCLCPP_INFO(this->get_logger(), "Index: %d", index);
      }
      if (index == durations.size()) {
        RCLCPP_INFO(this->get_logger(), "Writing durations...");
        for (uint32_t i = 0; i < index; i++) {
          f << durations[i] << std::endl;
        }
        RCLCPP_INFO(this->get_logger(), "Finished");
        f.flush();
        f.close();
      }
    } else if (message->index > 2) {
      RCLCPP_WARN(rclcpp::get_logger("eeg_trigger_processor"), "Unknown message index %d", message->index);
    }
  };

  trigger_subscription = this->create_subscription<eeg_interfaces::msg::Trigger>("/eeg/trigger_received",
                                                                                  10,
                                                                                  trigger_subscription_callback);

  this->signal_out_publisher = this->create_publisher<event_interfaces::msg::SignalOut>("/event/send/signal_out", 10);

  f.open(filename, std::ios::out | std::ios::trunc);

  durations = std::vector<double>(data_size, 0);
  RCLCPP_INFO(this->get_logger(), "durations size: %lu", durations.size());
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_trigger_processor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EEGTriggerProcessor>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_trigger_processor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);

  rclcpp::shutdown();
  return 0;
}
