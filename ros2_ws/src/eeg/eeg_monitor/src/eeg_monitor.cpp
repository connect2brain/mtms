#include <algorithm>
#include <chrono>
#include <cstdio>
#include <functional>
#include <memory>
#include <vector>

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
  TopicFrequency() : Node("eeg_monitor") {
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
      processing_times.push_back(message->processing_time);
    };

    eeg_preprocessed_subscription = this->create_subscription<eeg_interfaces::msg::PreprocessedEegSample>(
      EEG_PREPROCESSED_TOPIC,
      10,
      eeg_preprocessed_subscription_callback);

    timer = this->create_wall_timer(1000ms, std::bind(&TopicFrequency::timer_callback, this));
    RCLCPP_INFO(this->get_logger(), "Started timer");

  }

  void timer_callback() {
    RCLCPP_INFO(this->get_logger(), " ");
    RCLCPP_INFO(this->get_logger(), "EEG raw messages received during the last second: %d", eeg_raw_messages);
    RCLCPP_INFO(this->get_logger(), "EEG preprocessed messages received during the last second: %d", eeg_preprocessed_messages);

    if (!processing_times.empty()) {
      /* Maximum */
      double_t max_time = *std::max_element(processing_times.begin(), processing_times.end());

      /* 95% Percentile */
      size_t percentile_95_index = static_cast<size_t>(0.95 * processing_times.size());

      std::nth_element(processing_times.begin(), processing_times.begin() + percentile_95_index, processing_times.end());
      double_t percentile_95_time = processing_times[percentile_95_index];

      /* Median */
      size_t middle_index = processing_times.size() / 2;

      std::nth_element(processing_times.begin(), processing_times.begin() + middle_index, processing_times.end());
      double_t median_time = processing_times[middle_index];

      if(processing_times.size() % 2 == 0) {
        std::nth_element(processing_times.begin(), processing_times.begin() + middle_index - 1, processing_times.end());
        median_time = (median_time + processing_times[middle_index - 1]) / 2;
      }

      RCLCPP_INFO(this->get_logger(), " ");
      RCLCPP_INFO(this->get_logger(), "Max processing time during the last second: %.1f us", 1000000 * max_time);
      RCLCPP_INFO(this->get_logger(), "95%% percentile processing time during the last second: %.1f us", 1000000 * percentile_95_time);
      RCLCPP_INFO(this->get_logger(), "Median processing time during the last second: %.1f us", 1000000 * median_time);
      RCLCPP_INFO(this->get_logger(), " ");
    }

    eeg_raw_messages = 0;
    eeg_preprocessed_messages = 0;

    processing_times.clear();
  }


private:
  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Subscription<eeg_interfaces::msg::EegSample>::SharedPtr eeg_raw_subscription;
  rclcpp::Subscription<eeg_interfaces::msg::PreprocessedEegSample>::SharedPtr eeg_preprocessed_subscription;

  unsigned int eeg_raw_messages;
  unsigned int eeg_preprocessed_messages;
  std::vector<double_t> processing_times;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_monitor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<TopicFrequency>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_monitor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::executors::StaticSingleThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
