#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "eeg_interfaces/msg/eeg_sample.hpp"

#include "eeg_processor.h"
#include "processor_node.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

using namespace std::placeholders;

const std::string EEG_INFO_TOPIC = "/eeg/info";

EegProcessor::EegProcessor() : ProcessorNode("eeg_processor") {

  bool preprocess = true;

  this->declare_parameter<bool>("preprocess", true);
  this->get_parameter("preprocess", preprocess);

  this->eeg_topic = preprocess ? "/eeg/cleaned" : "/eeg/raw";

  RCLCPP_INFO(this->get_logger(), "Listening to EEG data on topic %s.", this->eeg_topic.c_str());

  this->charge_publisher = this->create_publisher<event_interfaces::msg::Charge>("/event/send/charge", 10);
  this->discharge_publisher = this->create_publisher<event_interfaces::msg::Discharge>("/event/send/discharge", 10);
  this->trigger_out_publisher = this->create_publisher<event_interfaces::msg::TriggerOut>("/event/send/trigger_out", 10);
  this->pulse_publisher = this->create_publisher<event_interfaces::msg::Pulse>("/event/send/pulse", 10);
  this->stimulus_publisher = this->create_publisher<event_interfaces::msg::Stimulus>("/event/send/stimulus", 10);

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
    std::bind(&EegProcessor::update_eeg_info, this, _1));

  /* Create subscription for EEG data. */

  auto eeg_subscription_callback = [this](const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) -> void {
    auto start = std::chrono::high_resolution_clock::now();

    RCLCPP_INFO_THROTTLE(this->get_logger(),
                         *this->get_clock(),
                         1000,
                         "Received EEG datapoint on topic %s with timestamp %.4f.",
                         this->eeg_topic.c_str(),
                         msg->time);

    this->handle_eeg_sample(msg);

    /* Print the time taken to process the datapoint. */

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    RCLCPP_DEBUG(this->get_logger(),
                 "Time taken to process EEG datapoint: %.3f ms.",
                 1000 * elapsed.count());
  };

  this->input_data_subscription = this->template create_subscription<eeg_interfaces::msg::EegSample>(this->eeg_topic, 5000,
                                                                                                        eeg_subscription_callback);

  /* Initialize variables. */

  this->previous_time = UNSET_PREVIOUS_TIME;
  this->sampling_frequency = UNSET_SAMPLING_FREQUENCY;
}

void EegProcessor::update_eeg_info(const std::shared_ptr<eeg_interfaces::msg::EegInfo> msg) {
  this->sampling_frequency = msg->sampling_frequency;
  this->sampling_period = 1.0 / this->sampling_frequency;

  RCLCPP_INFO(this->get_logger(), "Sampling frequency updated to %d Hz.", this->sampling_frequency);
}

/* XXX: Very close to a similar check in eeg_gatherer.cpp and other pipeline stages. Unify? */
void EegProcessor::check_dropped_samples(double_t current_time) {
  if (this->sampling_frequency == UNSET_SAMPLING_FREQUENCY) {
    RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Sampling frequency not received, cannot check for dropped samples.");
  }

  if (this->sampling_frequency != UNSET_SAMPLING_FREQUENCY &&
      this->previous_time) {

    auto time_diff = current_time - this->previous_time;
    auto threshold = this->sampling_period + this->TOLERANCE_S;

    if (time_diff > threshold) {
      /* Err if sample(s) were dropped. */
      RCLCPP_ERROR(rclcpp::get_logger("eeg_processor"),
          "Sample(s) dropped. Time difference between consecutive samples: %.5f, should be: %.5f, limit: %.5f", time_diff, this->sampling_period, threshold);

    } else {
      /* If log-level is set to DEBUG, print time difference for all samples, regardless of if samples were dropped or not. */
      RCLCPP_DEBUG(rclcpp::get_logger("eeg_processor"),
        "Time difference between consecutive samples: %.5f", time_diff);
    }
  }
  this->previous_time = current_time;
}

void EegProcessor::handle_eeg_sample(const std::shared_ptr<eeg_interfaces::msg::EegSample> msg) {
  auto current_time = msg->time;

  check_dropped_samples(current_time);

  auto events = processor->cleaned_eeg_received(*msg);
  publish_events(current_time, events);
}

void EegProcessor::publish_events(double_t time, const std::vector<Event> &events) {
  for (Event event: events) {
    RCLCPP_INFO(this->get_logger(), "Publishing events.");

    switch (event.event_type) {
      case PULSE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published pulse event timed at %.4f.",
                    event.pulse.event_info.execution_time);

        event.pulse.event_info.decision_time = time;
        this->pulse_publisher->publish(event.pulse);
        break;

      case CHARGE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published charge event timed at %.4f.",
                    event.charge.event_info.execution_time);

        event.charge.event_info.decision_time = time;
        this->charge_publisher->publish(event.charge);
        break;

      case DISCHARGE:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published discharge event timed at %.4f.",
                    event.discharge.event_info.execution_time);

        event.discharge.event_info.decision_time = time;
        this->discharge_publisher->publish(event.discharge);
        break;

      case TRIGGER_OUT:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published trigger out event timed at %.4f.",
                    event.trigger_out.event_info.execution_time);

        event.trigger_out.event_info.decision_time = time;
        this->trigger_out_publisher->publish(event.trigger_out);
        break;

      case STIMULUS:
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"),
                    "Published stimulus event timed at %.4f.",
                    event.stimulus.event_info.execution_time);

        event.stimulus.event_info.decision_time = time;
        this->stimulus_publisher->publish(event.stimulus);
        break;

      default:
        RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Warning, unknown event type: %d", event.event_type);
    }

  }
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegProcessor>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
