//
// Created by alqio on 16.1.2023.
//

#include "eeg_pre_processor.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

EegPreProcessor::EegPreProcessor() : ProcessorNode("eeg_pre_processor") {
  this->publisher = this->create_publisher<mtms_interfaces::msg::EegDatapoint>("/eeg/cleaned_data", 5000);

  auto subscription_callback = [this](const std::shared_ptr<mtms_interfaces::msg::EegDatapoint> message) -> void {
    auto samples = processor->raw_eeg_received(*message);
    publish_events(message->time, samples);
  };

  this->subscription = this->template create_subscription<mtms_interfaces::msg::EegDatapoint>("/eeg/raw_data", 5000,
                                                                                              subscription_callback);

}

void EegPreProcessor::publish_events(double_t time, const std::vector<mtms_interfaces::msg::EegDatapoint> &samples) {
  for (mtms_interfaces::msg::EegDatapoint sample: samples) {
    this->publisher->publish(sample);
  }
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_pre_processor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegPreProcessor>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_pre_processor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
