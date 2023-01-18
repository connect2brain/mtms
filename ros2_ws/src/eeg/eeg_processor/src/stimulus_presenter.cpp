//
// Created by alqio on 16.1.2023.
//

#include "event_interfaces/msg/stimulus.hpp"

#include "stimulus_presenter.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

StimulusPresenter::StimulusPresenter() : ProcessorNode("eeg_preprocessor") {
  this->charge_publisher = this->create_publisher<event_interfaces::msg::Charge>("/event/charge", 10);
  this->discharge_publisher = this->create_publisher<event_interfaces::msg::Discharge>("/event/discharge", 10);
  this->signal_out_publisher = this->create_publisher<event_interfaces::msg::SignalOut>("/event/signal_out", 10);
  this->pulse_publisher = this->create_publisher<event_interfaces::msg::Pulse>("/event/pulse", 10);


  auto subscription_callback = [this](const std::shared_ptr<event_interfaces::msg::Stimulus> message) -> void {
    auto events = processor->present_stimulus_received(*message);
    publish_events(message->event_info.decision_time, events);
  };

  this->subscription = this->template create_subscription<event_interfaces::msg::Stimulus>("/event/stimulus", 5000,
                                                                                           subscription_callback);

}

void StimulusPresenter::publish_events(double_t time, const std::vector<Event> &events) {
  for (Event event: events) {
    RCLCPP_INFO(this->get_logger(), "Publishing events.");

    switch (event.event_type) {
      case PULSE:
        RCLCPP_INFO(rclcpp::get_logger("stimulus_presenter"),
                    "Published pulse event timed at %.4f.",
                    event.pulse.event_info.execution_time);

        event.pulse.event_info.decision_time = time;
        this->pulse_publisher->publish(event.pulse);
        break;

      case CHARGE:
        RCLCPP_INFO(rclcpp::get_logger("stimulus_presenter"),
                    "Published charge event timed at %.4f.",
                    event.charge.event_info.execution_time);

        event.charge.event_info.decision_time = time;
        this->charge_publisher->publish(event.charge);
        break;

      case DISCHARGE:
        RCLCPP_INFO(rclcpp::get_logger("stimulus_presenter"),
                    "Published discharge event timed at %.4f.",
                    event.discharge.event_info.execution_time);

        event.discharge.event_info.decision_time = time;
        this->discharge_publisher->publish(event.discharge);
        break;

      case SIGNAL_OUT:
        RCLCPP_INFO(rclcpp::get_logger("stimulus_presenter"),
                    "Published signal out event timed at %.4f.",
                    event.signal_out.event_info.execution_time);

        event.signal_out.event_info.decision_time = time;
        this->signal_out_publisher->publish(event.signal_out);
        break;

      case STIMULUS:
        RCLCPP_WARN(rclcpp::get_logger("stimulus_presenter"),
                    "Warning, sending stimulus from stimulus presenter is not supported");

      default:
        RCLCPP_WARN(rclcpp::get_logger("stimulus_presenter"), "Warning, unknown event type: %d", event.event_type);
    }

  }
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_preprocessor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StimulusPresenter>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_preprocessor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
