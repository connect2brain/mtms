#include "rclcpp/rclcpp.hpp"

#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "mtms_interfaces/msg/event.hpp"

#include "eeg_processor.h"
#include "processor_node.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

EegProcessor::EegProcessor() : ProcessorNode("eeg_processor") {

  this->charge_publisher = this->create_publisher<fpga_interfaces::msg::Charge>("/mtms/charge", 10);
  this->discharge_publisher = this->create_publisher<fpga_interfaces::msg::Discharge>("/mtms/discharge", 10);
  this->signal_out_publisher = this->create_publisher<fpga_interfaces::msg::SignalOut>("/mtms/signal_out", 10);
  this->pulse_publisher = this->create_publisher<fpga_interfaces::msg::Pulse>("/mtms/pulse", 10);

  auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
    auto events = processor->eeg_received(*message);
    publish_events(message->time, events);
  };

  this->subscription = this->template create_subscription<mtms_interfaces::msg::EegDatapoint>("/eeg/cleaned_data", 5000,
                                                                            subscription_callback);

}

void EegProcessor::publish_events(double_t time, const std::vector<Event> &events) {
  for (Event event: events) {
    RCLCPP_INFO(this->get_logger(), "Publishing events.");
/*
    switch (event.event_type) {
      case PULSE:
        fpga_interfaces::msg::Pulse ros_event;
        ros_event.event_type = event.event_type;
        ros_event.processing_start_time = time;
        ros_event.when_to_execute = event.pulse.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published fpga pulse event timed at %.4f.",
                    ros_event.when_to_execute);
        this->pulse_publisher(ros_event);

        break;

      case CHARGE:
        fpga_interfaces::msg::Charge ros_event;
        ros_event.event_type = event.event_type;
        ros_event.processing_start_time = time;
        ros_event.when_to_execute = event.charge.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published charge event timed at %.4f.",
                    ros_event.when_to_execute);
        break;

      case DISCHARGE:
        fpga_interfaces::msg::Discharge ros_event;
        ros_event.event_type = event.event_type;
        ros_event.processing_start_time = time;
        ros_event.when_to_execute = event.discharge.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published discharge event timed at %.4f.",
                    ros_event.when_to_execute);
        break;

      case SIGNAL_OUT:
        fpga_interfaces::msg::SignalOut ros_event;
        ros_event.event_type = event.event_type;
        ros_event.processing_start_time = time;
        ros_event.when_to_execute = event.signal_out.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published signal out event timed at %.4f.",
                    ros_event.when_to_execute);
        break;

      default:
        RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Warning, unknown fpga event type: %d", event.event_type);
    }
*/
  }
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
  rclcpp::shutdown();

  return 0;
}
