#include "rclcpp/rclcpp.hpp"

#include "mtms_interfaces/msg/eeg_datapoint.hpp"

#include "eeg_processor.h"
#include "processor_node.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

EegProcessor::EegProcessor() : ProcessorNode("eeg_processor") {

  this->charge_publisher = this->create_publisher<event_interfaces::msg::Charge>("/event/charge", 10);
  this->discharge_publisher = this->create_publisher<event_interfaces::msg::Discharge>("/event/discharge", 10);
  this->signal_out_publisher = this->create_publisher<event_interfaces::msg::SignalOut>("/event/signal_out", 10);
  this->pulse_publisher = this->create_publisher<event_interfaces::msg::Pulse>("/event/pulse", 10);
  this->stimulus_publisher = this->create_publisher<event_interfaces::msg::Stimulus>("/event/stimulus", 10);

  auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
    auto events = processor->cleaned_eeg_received(*message);
    publish_events(message->time, events);
  };

  this->subscription = this->template create_subscription<mtms_interfaces::msg::EegDatapoint>("/eeg/cleaned_data", 5000,
                                                                            subscription_callback);

}

void EegProcessor::publish_events(double_t time, const std::vector<Event> &events) {
  for (Event event: events) {
    RCLCPP_INFO(this->get_logger(), "Publishing events.");

    switch (event.event_type) {
      case PULSE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published fpga pulse event timed at %.4f.",
                    event.pulse.event_info.execution_time);

        event.pulse.event_info.decision_time = time;
        this->pulse_publisher->publish(event.pulse);
        break;

      case CHARGE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published fpga charge event timed at %.4f.",
                    event.charge.event_info.execution_time);

        event.charge.event_info.decision_time = time;
        this->charge_publisher->publish(event.charge);
        break;

      case DISCHARGE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published fpga discharge event timed at %.4f.",
                    event.discharge.event_info.execution_time);

        event.discharge.event_info.decision_time = time;
        this->discharge_publisher->publish(event.discharge);
        break;

      case SIGNAL_OUT:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published signal out pulse event timed at %.4f.",
                    event.signal_out.event_info.execution_time);

        event.signal_out.event_info.decision_time = time;
        this->signal_out_publisher->publish(event.signal_out);
        break;

      case STIMULUS:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published stimulus event timed at %.4f.",
                    event.stimulus.event_info.execution_time);

        event.stimulus.event_info.decision_time = time;
        this->stimulus_publisher->publish(event.stimulus);
        break;

      default:
        RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Warning, unknown fpga event type: %d", event.event_type);
    }

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
