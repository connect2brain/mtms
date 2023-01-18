//
// Created by alqio on 16.1.2023.
//

#include "eeg_preprocessor.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

EegPreprocessor::EegPreprocessor() : ProcessorNode("eeg_preprocessor") {
  this->publisher = this->create_publisher<eeg_interfaces::msg::EegDatapoint>("/eeg/cleaned_data", 5000);

  auto subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegDatapoint> message) -> void {
    auto samples = processor->raw_eeg_received(*message);
    publish_events(message->time, samples);
  };

  this->subscription = this->template create_subscription<eeg_interfaces::msg::EegDatapoint>("/eeg/raw_data", 5000,
                                                                                              subscription_callback);

}

void EegPreprocessor::publish_events(double_t time, const std::vector<eeg_interfaces::msg::EegDatapoint> &samples) {
  for (eeg_interfaces::msg::EegDatapoint sample: samples) {
    this->publisher->publish(sample);
  }
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_preprocessor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegPreprocessor>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_preprocessor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
