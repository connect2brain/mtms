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
#include "eeg_interfaces/msg/eeg_statistics.hpp"

using namespace std::chrono_literals;

const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_PREPROCESSED_TOPIC = "/eeg/preprocessed";
const std::string EEG_STATISTICS_TOPIC = "/eeg/statistics";

class EegMonitor : public rclcpp::Node {

public:
  EegMonitor() : Node("eeg_monitor") {
    num_of_raw_eeg_samples = 0;

    /* Subscriber for raw EEG. */
    auto eeg_raw_subscriber_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegSample> message) -> void {
      num_of_raw_eeg_samples++;
    };

    eeg_raw_subscriber = this->create_subscription<eeg_interfaces::msg::EegSample>(
      EEG_RAW_TOPIC,
      10,
      eeg_raw_subscriber_callback);

    /* Subscriber for preprocessed EEG. */
    auto eeg_preprocessed_subscriber_callback = [this](const std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample> message) -> void {
      num_of_preprocessed_eeg_samples++;
      processing_times.push_back(message->processing_time);
    };

    eeg_preprocessed_subscriber = this->create_subscription<eeg_interfaces::msg::PreprocessedEegSample>(
      EEG_PREPROCESSED_TOPIC,
      10,
      eeg_preprocessed_subscriber_callback);

    /* Publisher for statistics. */
    eeg_statistics_publisher = this->create_publisher<eeg_interfaces::msg::EegStatistics>(
      EEG_STATISTICS_TOPIC,
      10);

    /* Timer for computing statistics for each second. */
    timer = this->create_wall_timer(1000ms, std::bind(&EegMonitor::timer_callback, this));
    RCLCPP_INFO(this->get_logger(), "Started timer");
  }

  void timer_callback() {
    double max_time = 0.0;
    double q95_time = 0.0;
    double median_time = 0.0;

    if (!processing_times.empty()) {
      /* Maximum */
      max_time = *std::max_element(processing_times.begin(), processing_times.end());

      /* 95% Percentile */
      size_t q95_index = static_cast<size_t>(0.95 * processing_times.size());

      std::nth_element(processing_times.begin(), processing_times.begin() + q95_index, processing_times.end());
      q95_time = processing_times[q95_index];

      /* Median */
      size_t middle_index = processing_times.size() / 2;

      std::nth_element(processing_times.begin(), processing_times.begin() + middle_index, processing_times.end());
      median_time = processing_times[middle_index];

      if(processing_times.size() % 2 == 0) {
        std::nth_element(processing_times.begin(), processing_times.begin() + middle_index - 1, processing_times.end());
        median_time = (median_time + processing_times[middle_index - 1]) / 2;
      }
    }

    /* Publish the statistics. */
    auto msg = eeg_interfaces::msg::EegStatistics();

    msg.num_of_raw_samples = num_of_raw_eeg_samples;
    msg.num_of_preprocessed_samples = num_of_preprocessed_eeg_samples;
    msg.preprocessing_time_max = max_time;
    msg.preprocessing_time_q95 = q95_time;
    msg.preprocessing_time_median = median_time;

    eeg_statistics_publisher->publish(msg);

    /* Reset counters. */
    num_of_raw_eeg_samples = 0;
    num_of_preprocessed_eeg_samples = 0;

    processing_times.clear();
  }


private:
  rclcpp::Subscription<eeg_interfaces::msg::EegSample>::SharedPtr eeg_raw_subscriber;
  rclcpp::Subscription<eeg_interfaces::msg::PreprocessedEegSample>::SharedPtr eeg_preprocessed_subscriber;
  rclcpp::Publisher<eeg_interfaces::msg::EegStatistics>::SharedPtr eeg_statistics_publisher;

  rclcpp::TimerBase::SharedPtr timer;

  unsigned int num_of_raw_eeg_samples;
  unsigned int num_of_preprocessed_eeg_samples;
  std::vector<double_t> processing_times;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_monitor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegMonitor>();

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
