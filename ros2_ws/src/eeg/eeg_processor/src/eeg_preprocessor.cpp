//
// Created by alqio on 16.1.2023.
//

#include <chrono>

#include "eeg_preprocessor.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

using namespace std::placeholders;

const std::string EEG_INFO_TOPIC = "/eeg/info";
const std::string EEG_RAW_TOPIC = "/eeg/raw";
const std::string EEG_CLEANED_TOPIC = "/eeg/cleaned";

EegPreprocessor::EegPreprocessor() : ProcessorNode("eeg_preprocessor") {
  this->cleaned_eeg_publisher = this->create_publisher<eeg_interfaces::msg::EegSample>(EEG_CLEANED_TOPIC, 5000);

  /* Create subscription for EEG info. */

  const rmw_qos_profile_t qos_profile_persist_latest = {
      RMW_QOS_POLICY_HISTORY_KEEP_LAST,
      1,
      RMW_QOS_POLICY_RELIABILITY_RELIABLE,
      RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
      RMW_QOS_DEADLINE_DEFAULT,
      RMW_QOS_LIFESPAN_DEFAULT,
      RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT,
      RMW_QOS_LIVELINESS_LEASE_DURATION_DEFAULT,
      false
  };

  auto qos_persist_latest = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile_persist_latest.history, qos_profile_persist_latest.depth), qos_profile_persist_latest);

  this->eeg_info_subscription = this->create_subscription<eeg_interfaces::msg::EegInfo>(
    EEG_INFO_TOPIC,
    qos_persist_latest,
    std::bind(&EegPreprocessor::update_eeg_info, this, _1));

  /* Create subscription for EEG data. */

  auto eeg_subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) -> void {
    auto start = std::chrono::high_resolution_clock::now();

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Received EEG datapoint on topic %s with timestamp %.4f.",
                         EEG_RAW_TOPIC.c_str(),
                         msg->time);

    this->handle_eeg_sample(msg);

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Published cleaned EEG data on topic %s",
                         EEG_CLEANED_TOPIC.c_str());

    /* Print the time taken to preprocess the datapoint. */

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    RCLCPP_DEBUG(this->get_logger(),
                 "Time taken to preprocess EEG datapoint: %.3f ms.",
                 1000 * elapsed.count());
  };

  this->input_data_subscription = this->template create_subscription<eeg_interfaces::msg::EegSample>(EEG_RAW_TOPIC, 5000,
                                                                                                        eeg_subscription_callback);
  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", EEG_RAW_TOPIC.c_str());

  /* Initialize variables. */

  this->previous_time = UNSET_PREVIOUS_TIME;
  this->sampling_frequency = UNSET_SAMPLING_FREQUENCY;
}

void EegPreprocessor::update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg) {
  this->sampling_frequency = msg->sampling_frequency;
  this->sampling_period = 1.0 / this->sampling_frequency;

  RCLCPP_INFO(this->get_logger(), "Sampling frequency updated to %d Hz.", this->sampling_frequency);
}

/* XXX: Very close to a similar check in eeg_gatherer.cpp and other pipeline stages. Unify? */
void EegPreprocessor::check_dropped_samples(double_t current_time) {
  if (this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {
    RCLCPP_WARN(rclcpp::get_logger("eeg_preprocessor"), "Sampling frequency not received, cannot check for dropped samples.");
  }

  if (this->sampling_frequency != UNSET_SAMPLING_FREQUENCY &&
      this->previous_time) {

    auto time_diff = current_time - this->previous_time;
    auto threshold = this->sampling_period + this->TOLERANCE_S;

    if (time_diff > threshold) {
      /* Err if sample(s) were dropped. */
      RCLCPP_ERROR(rclcpp::get_logger("eeg_preprocessor"),
          "Sample(s) dropped. Time difference between consecutive samples: %.5f, should be: %.5f, limit: %.5f", time_diff, this->sampling_period, threshold);

    } else {
      /* If log-level is set to DEBUG, print time difference for all samples, regardless of if samples were dropped or not. */
      RCLCPP_DEBUG(rclcpp::get_logger("eeg_preprocessor"),
        "Time difference between consecutive samples: %.5f", time_diff);
    }
  }
  this->previous_time = current_time;
}

void EegPreprocessor::handle_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) {
  auto current_time = msg->time;

  check_dropped_samples(current_time);

  auto samples = processor->raw_eeg_received(*msg);
  publish_cleaned_eeg(current_time, samples);
}

void
EegPreprocessor::publish_cleaned_eeg(double_t time, const std::vector<eeg_interfaces::msg::EegSample> &samples) {
  for (eeg_interfaces::msg::EegSample sample: samples) {
    this->cleaned_eeg_publisher->publish(sample);
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
