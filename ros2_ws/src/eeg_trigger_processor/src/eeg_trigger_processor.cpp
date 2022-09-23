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

  auto trigger_subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::Trigger> message) -> void {
    if (message->index == 1) {
      trigger_out_client->async_send_request(req);
    } else if (message->index == 2) {

      if (index < durations.size()) {
        durations[index++] = message->time_us;
      } else {
        index++;
      }

      //RCLCPP_INFO(this->get_logger(), "index: %d", index);
      if (index % 1000 == 0) {
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
      //RCLCPP_INFO(this->get_logger(), "Difference between triggers: %lu", message->time_us);
    } else {
      RCLCPP_WARN(rclcpp::get_logger("eeg_trigger_processor"), "Unknown message index %d", message->index);
    }
  };

  static const rmw_qos_profile_t qos_profile = {
      RMW_QOS_POLICY_HISTORY_KEEP_LAST,
      1,
      RMW_QOS_POLICY_RELIABILITY_RELIABLE,
      RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
      RMW_QOS_DEADLINE_DEFAULT,
      RMW_QOS_LIFESPAN_DEFAULT,
      RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT,
      RMW_QOS_LIVELINESS_LEASE_DURATION_DEFAULT,
      false
  };

  auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, qos_profile.depth), qos_profile);


  trigger_subscription = this->create_subscription<mtms_interfaces::msg::Trigger>("/eeg/trigger_received",
                                                                                  qos,
                                                                                  trigger_subscription_callback);

  trigger_out_client = this->create_client<fpga_interfaces::srv::SendTriggerOutEvent>("/fpga/send_trigger_out_event",
                                                                                      qos_profile);

  req = std::make_shared<fpga_interfaces::srv::SendTriggerOutEvent::Request>();
  auto event = fpga_interfaces::msg::TriggerOutEvent();
  event.index = 3;
  event.duration_us = 10000;
  event.event_info.time_us = 0;
  event.event_info.execution_condition = 2;
  event.event_info.event_id = 1;

  req->trigger_out_event = event;

  f.open(filename, std::ios::out | std::ios::trunc);

  durations = std::vector<double>(data_size, 0);
  RCLCPP_INFO(this->get_logger(), "durations size: %lu", durations.size());

}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);


#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_trigger_processor"), "Setting thread scheduling and memory lock");
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
