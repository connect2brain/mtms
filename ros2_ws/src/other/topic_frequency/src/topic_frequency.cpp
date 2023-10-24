#include <chrono>
#include <functional>
#include <memory>
#include <cstdio>
#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/preprocessed_eeg_sample.hpp"

using namespace std::chrono_literals;

const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";

class TopicFrequency : public rclcpp::Node {

public:
  TopicFrequency() : Node("topic_frequency") {
    eeg_raw_messages = 0;

    /* Raw EEG */
    auto eeg_raw_subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegSample> message) -> void {
      eeg_raw_messages++;
    };

    eeg_raw_subscription = this->create_subscription<eeg_interfaces::msg::EegSample>(
      EEG_RAW_TOPIC,
      10,
      eeg_raw_subscription_callback);

    /* Preprocessed EEG */
    auto eeg_preprocessed_subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample> message) -> void {
      eeg_preprocessed_messages++;
    };

    eeg_preprocessed_subscription = this->create_subscription<eeg_interfaces::msg::PreprocessedEegSample>(
      EEG_PREPROCESSED_TOPIC,
      10,
      eeg_preprocessed_subscription_callback);

    timer = this->create_wall_timer(1000ms, std::bind(&TopicFrequency::timer_callback, this));
    RCLCPP_INFO(this->get_logger(), "Started timer");

  }

  void timer_callback() {
    RCLCPP_INFO(this->get_logger(), "EEG raw messages received during the last second: %d", eeg_raw_messages);
    RCLCPP_INFO(this->get_logger(), "EEG preprocessed messages received during the last second: %d", eeg_preprocessed_messages);
    eeg_raw_messages = 0;
    eeg_preprocessed_messages = 0;
  }


private:
  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Subscription<eeg_interfaces::msg::EegSample>::SharedPtr eeg_raw_subscription;
  rclcpp::Subscription<eeg_interfaces::msg::PreprocessedEegSample>::SharedPtr eeg_preprocessed_subscription;
  unsigned int eeg_raw_messages;
  unsigned int eeg_preprocessed_messages;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("topic_frequency"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<TopicFrequency>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("topic_frequency"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::executors::StaticSingleThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
