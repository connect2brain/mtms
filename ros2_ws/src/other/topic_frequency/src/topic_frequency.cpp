#include <chrono>
#include <functional>
#include <memory>
#include <cstdio>
#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "eeg_interfaces/msg/eeg_datapoint.hpp"

using namespace std::chrono_literals;


class TopicFrequency : public rclcpp::Node {

public:
  TopicFrequency() : Node("topic_frequency") {

    messages_received_since_last_full_second = 0;
    messages_received = 0;

    auto subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegDatapoint> message) -> void {
      messages_received++;
      messages_received_since_last_full_second++;
    };

    subscription = this->create_subscription<eeg_interfaces::msg::EegDatapoint>("/eeg/raw",
                                                                                 10,
                                                                                 subscription_callback);

    timer = this->create_wall_timer(1000ms, std::bind(&TopicFrequency::timer_callback, this));
    RCLCPP_INFO(this->get_logger(), "Started timer");

  }

  void timer_callback() {
    RCLCPP_INFO(this->get_logger(), "Messages received during the last second: %d, total messages: %d", messages_received_since_last_full_second, messages_received);
    messages_received_since_last_full_second = 0;
  }


private:
  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Subscription<eeg_interfaces::msg::EegDatapoint>::SharedPtr subscription;
  unsigned int messages_received;
  unsigned int messages_received_since_last_full_second;
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
