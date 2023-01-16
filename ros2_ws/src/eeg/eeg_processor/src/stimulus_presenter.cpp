//
// Created by alqio on 16.1.2023.
//

#include "stimulus_presenter.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

StimulusPresenter::StimulusPresenter() : ProcessorNode("eeg_pre_processor") {
  this->charge_publisher = this->create_publisher<event_interfaces::msg::Charge>("/event/charge", 10);
  this->discharge_publisher = this->create_publisher<event_interfaces::msg::Discharge>("/event/discharge", 10);
  this->signal_out_publisher = this->create_publisher<event_interfaces::msg::SignalOut>("/event/signal_out", 10);
  this->pulse_publisher = this->create_publisher<event_interfaces::msg::Pulse>("/event/pulse", 10);


  auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::Event> message) -> void {
    auto events = processor->present_stimulus_received(*message);
    publish_events(message->when_to_execute, events);
  };

  this->subscription = this->template create_subscription<mtms_interfaces::msg::Event>("/eeg/cleaned_data", 5000,
                                                                                              subscription_callback);

}

void StimulusPresenter::publish_events(double_t time, const std::vector<Event> &events) {
  for (Event event : events) {

  }
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_pre_processor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StimulusPresenter>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_pre_processor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
