//
// Created by alqio on 16.1.2023.
//

#include "eeg_preprocessor.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_CLEANED_TOPIC = "/eeg/cleaned";

EegPreprocessor::EegPreprocessor() : ProcessorNode("eeg_preprocessor") {
  this->publisher = this->create_publisher<eeg_interfaces::msg::EegDatapoint>(EEG_CLEANED_TOPIC, 5000);

  auto eeg_subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegDatapoint> message) -> void {
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Received EEG data on topic %s", EEG_RAW_TOPIC.c_str());

    auto samples = processor->raw_eeg_received(*message);
    publish_cleaned_eeg(message->time, samples);

    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Published EEG data on topic %s", EEG_CLEANED_TOPIC.c_str());
  };

  this->input_data_subscription = this->template create_subscription<eeg_interfaces::msg::EegDatapoint>(EEG_RAW_TOPIC, 5000,
                                                                                                        eeg_subscription_callback);
  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", EEG_RAW_TOPIC.c_str());
}

void
EegPreprocessor::publish_cleaned_eeg(double_t time, const std::vector<eeg_interfaces::msg::EegDatapoint> &samples) {
  for (eeg_interfaces::msg::EegDatapoint sample: samples) {
    this->publisher->publish(sample);
  }
}

void EegPreprocessor::publish_events(double_t time, const std::vector<Event> &events) {
  RCLCPP_INFO(rclcpp::get_logger("eeg_preprocessor"), "Ignoring publish_events on EEG preprocessor.");
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
