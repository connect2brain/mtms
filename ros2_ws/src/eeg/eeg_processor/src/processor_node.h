/*
 Created by alqio on 16.1.2023.
 The definition of the methods should also be here in the header,
 read more here why: https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor
 */


#ifndef EEG_PROCESSOR_PROCESSOR_NODE_H
#define EEG_PROCESSOR_PROCESSOR_NODE_H

#include <string>

#include "rclcpp/rclcpp.hpp"

#include "processor.h"
#include "compiled_matlab_processor/compiled_matlab_processor.h"
#include "cpp_processor/cpp_processor.h"

#include "mtms_device_interfaces/msg/system_state.hpp"

#if defined(MATLAB_FOUND)

#include "matlab_processor/matlab_processor.h"

#endif

#if defined(PYTHON_FOUND)

#include "python_processor/python_processor.h"

#endif

using namespace std::chrono;
using namespace std::chrono_literals;
/* HACK: Needs to match the values in system_state_bridge.cpp. */
const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL = 20ms;
const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE = 5ms;

template<class SubscriptionType, class OutputType>
class ProcessorNode : public rclcpp::Node {
public:
  ProcessorNode(std::string node_name);

  virtual void publish_events(double_t time, const std::vector<OutputType> &events) = 0;

  void load_processor_script(std::string processor_type, std::string processor_script_path);

  typename rclcpp::Subscription<SubscriptionType>::SharedPtr subscription;

  ProcessorWrapper *processor;

  rclcpp::Subscription<mtms_device_interfaces::msg::SystemState>::SharedPtr system_state_subscription;
  mtms_device_interfaces::msg::ExperimentState experiment_state;
  rclcpp::Subscription<mtms_device_interfaces::msg::SystemState>::SharedPtr subscription_system_state;

};


template<class SubscriptionType, class OutputType>
ProcessorNode<SubscriptionType, OutputType>::ProcessorNode(std::string node_name) : Node(
    node_name) {
  std::string processor_type;
  this->declare_parameter<std::string>("processor_type", "cpp");
  this->get_parameter("processor_type", processor_type);

  std::string processor_script_path;
  this->declare_parameter<std::string>("processor_script", "");
  this->get_parameter("processor_script", processor_script_path);

  this->load_processor_script(processor_type, processor_script_path);

  auto system_state_callback = [this](const std::shared_ptr<mtms_device_interfaces::msg::SystemState> message) -> void {
    if (message->experiment_state.value == mtms_device_interfaces::msg::ExperimentState::STOPPED &&
        experiment_state.value != mtms_device_interfaces::msg::ExperimentState::STOPPED) {

      //This function was removed earlier, but should be added back.
      //this->processor->end_experiment();
    }

    if (message->experiment_state.value == mtms_device_interfaces::msg::ExperimentState::STARTED &&
        experiment_state.value != mtms_device_interfaces::msg::ExperimentState::STARTED) {

      this->processor->init();

    }

    experiment_state = message->experiment_state;

  };

  /* HACK: Duplicates code from system_state_bridge.cpp. */
  auto deadline = SYSTEM_STATE_PUBLISHING_INTERVAL + SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE;
  const uint64_t deadline_ns = static_cast<uint64_t>(std::chrono::nanoseconds(deadline).count());
  const rmw_time_t rmw_deadline = {0, deadline_ns};
  const rmw_time_t rmw_lifespan = rmw_deadline;

  const rmw_qos_profile_t qos_profile = {
      RMW_QOS_POLICY_HISTORY_KEEP_LAST,
      1,
      RMW_QOS_POLICY_RELIABILITY_RELIABLE,
      RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
      rmw_deadline,
      rmw_lifespan,
      RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT,
      RMW_QOS_LIVELINESS_LEASE_DURATION_DEFAULT,
      false
  };
  auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, qos_profile.depth), qos_profile);

  rclcpp::SubscriptionOptions subscription_options;
  subscription_options.event_callbacks.deadline_callback = [this](
      [[maybe_unused]] rclcpp::QOSDeadlineRequestedInfo &event) -> void {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "System state not received within deadline.");
  };

  this->subscription_system_state = this->create_subscription<mtms_device_interfaces::msg::SystemState>(
      "/mtms_device/system_state", qos,
      system_state_callback, subscription_options);
}

template<class SubscriptionType, class OutputType>
void ProcessorNode<SubscriptionType, OutputType>::load_processor_script(std::string processor_type,
                                                                        std::string processor_script_path) {
  if (processor_type == "python") {
#ifdef PYTHON_FOUND
    processor = new PythonProcessor(processor_script_path);
#else
    RCLCPP_ERROR(this->get_logger(), "ERROR: Trying to use python processor but compiled without python!");
#endif

  } else if (processor_type == "matlab") {
#ifdef MATLAB_FOUND
    processor = new MatlabProcessor(processor_script_path);
#else
    RCLCPP_ERROR(this->get_logger(), "ERROR: Trying to use MATLAB processor but compiled without MATLAB!");
#endif

  } else if (processor_type == "compiledmatlab") {
    processor = new CompiledMatlabProcessor(processor_script_path);

  } else if (processor_type == "cpp") {
    processor = new CppProcessor(processor_script_path);

  } else {
    RCLCPP_ERROR(this->get_logger(), "No processor specified!");
  }

}


#endif //EEG_PROCESSOR_PROCESSOR_NODE_H
