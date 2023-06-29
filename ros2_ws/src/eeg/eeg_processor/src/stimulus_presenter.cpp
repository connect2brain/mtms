//
// Created by alqio on 16.1.2023.
//

#include "event_interfaces/msg/stimulus.hpp"

#include "stimulus_presenter.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

StimulusPresenter::StimulusPresenter() : ProcessorNode("stimulus_presenter") {
  this->charge_publisher = this->create_publisher<event_interfaces::msg::Charge>("/event/send/charge", 10);
  this->discharge_publisher = this->create_publisher<event_interfaces::msg::Discharge>("/event/send/discharge", 10);
  this->trigger_out_publisher = this->create_publisher<event_interfaces::msg::TriggerOut>("/event/send/trigger_out", 10);
  this->pulse_publisher = this->create_publisher<event_interfaces::msg::Pulse>("/event/send/pulse", 10);


  auto subscription_callback = [this](const std::shared_ptr<event_interfaces::msg::Stimulus> message) -> void {

    /* If immediate event, publish it immediately. Otherwise, add it to a buffer and wait until its time arrives. */
    if (message->event_info.execution_condition.value == event_interfaces::msg::ExecutionCondition::IMMEDIATE) {
      auto events = this->processor->present_stimulus_received(*message);
      publish_events(message->event_info.decision_time, events);
    } else {
      this->event_buffer.push_back(*message);
    }

  };

  this->subscription = this->template create_subscription<event_interfaces::msg::Stimulus>("/event/send/stimulus", 5000,
                                                                                           subscription_callback);

  auto eeg_subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegDatapoint> message) -> void {
    std::vector<unsigned> ids_to_remove;

    for (unsigned i = 0; i < this->event_buffer.size(); i++) {

      auto event = this->event_buffer[i];

      /* If current time >= the time the event was supposed to be executed at, execute it and remove it from buffer. 
         Otherwise, keep it in the buffer. */
      if (message->time >= event.event_info.execution_time) {
        auto events = this->processor->present_stimulus_received(event);
        publish_events(event.event_info.decision_time, events);
        ids_to_remove.push_back(i);
      }
    }

    /* Erase executed events from the buffer. Not performed simultaneously with the previous loop to not affect the 
       loop indices. */
    for (auto id_to_remove: ids_to_remove) {
      this->event_buffer.erase(this->event_buffer.begin() + id_to_remove);
    }
  };

  this->eeg_subscription = this->create_subscription<eeg_interfaces::msg::EegDatapoint>("/eeg/cleaned", 5000,
                                                                                        eeg_subscription_callback);

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

      case TRIGGER_OUT:
        RCLCPP_INFO(rclcpp::get_logger("stimulus_presenter"),
                    "Published trigger out event timed at %.4f.",
                    event.trigger_out.event_info.execution_time);

        event.trigger_out.event_info.decision_time = time;
        this->trigger_out_publisher->publish(event.trigger_out);
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
  RCLCPP_INFO(rclcpp::get_logger("stimulus_presenter"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StimulusPresenter>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("stimulus_presenter"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
