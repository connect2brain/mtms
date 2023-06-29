#include "rclcpp/rclcpp.hpp"

#include "eeg_interfaces/msg/eeg_datapoint.hpp"

#include "eeg_processor.h"
#include "processor_node.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

EegProcessor::EegProcessor() : ProcessorNode("eeg_processor") {

  bool preprocess = true;

  this->declare_parameter<bool>("preprocess", true);
  this->get_parameter("preprocess", preprocess);

  this->eeg_topic = preprocess ? "/eeg/cleaned" : "/eeg/raw";

  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", this->eeg_topic.c_str());

  this->charge_publisher = this->create_publisher<event_interfaces::msg::Charge>("/event/send/charge", 10);
  this->discharge_publisher = this->create_publisher<event_interfaces::msg::Discharge>("/event/send/discharge", 10);
  this->trigger_out_publisher = this->create_publisher<event_interfaces::msg::TriggerOut>("/event/send/trigger_out", 10);
  this->pulse_publisher = this->create_publisher<event_interfaces::msg::Pulse>("/event/send/pulse", 10);
  this->stimulus_publisher = this->create_publisher<event_interfaces::msg::Stimulus>("/event/send/stimulus", 10);

  auto subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegDatapoint> message) -> void {
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Received EEG data on topic %s", this->eeg_topic.c_str());

    auto events = processor->cleaned_eeg_received(*message);
    publish_events(message->time, events);
  };

  this->subscription = this->template create_subscription<eeg_interfaces::msg::EegDatapoint>(this->eeg_topic, 5000,
                                                                                             subscription_callback);

}

void EegProcessor::publish_events(double_t time, const std::vector<Event> &events) {
  for (Event event: events) {
    RCLCPP_INFO(this->get_logger(), "Publishing events.");

    switch (event.event_type) {
      case PULSE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published pulse event timed at %.4f.",
                    event.pulse.event_info.execution_time);

        event.pulse.event_info.decision_time = time;
        this->pulse_publisher->publish(event.pulse);
        break;

      case CHARGE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published charge event timed at %.4f.",
                    event.charge.event_info.execution_time);

        event.charge.event_info.decision_time = time;
        this->charge_publisher->publish(event.charge);
        break;

      case DISCHARGE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published discharge event timed at %.4f.",
                    event.discharge.event_info.execution_time);

        event.discharge.event_info.decision_time = time;
        this->discharge_publisher->publish(event.discharge);
        break;

      case TRIGGER_OUT:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published trigger out event timed at %.4f.",
                    event.trigger_out.event_info.execution_time);

        event.trigger_out.event_info.decision_time = time;
        this->trigger_out_publisher->publish(event.trigger_out);
        break;

      case STIMULUS:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published stimulus event timed at %.4f.",
                    event.stimulus.event_info.execution_time);

        event.stimulus.event_info.decision_time = time;
        this->stimulus_publisher->publish(event.stimulus);
        break;

      default:
        RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Warning, unknown event type: %d", event.event_type);
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
