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

#if defined(MATLAB_FOUND)

#include "matlab_processor/matlab_processor.h"

#endif

#if defined(PYTHON_FOUND)

#include "python_processor/python_processor.h"

#endif


template<class SubscriptionType, class PublisherType>
class ProcessorNode : public rclcpp::Node {
public:
  ProcessorNode(std::string node_name, std::string subscription_topic, std::string publisher_topic);

  ~ProcessorNode() {};

private:
  typename rclcpp::Subscription<SubscriptionType>::SharedPtr subscription;
  typename rclcpp::Publisher<PublisherType>::SharedPtr publisher;

  ProcessorWrapper *processor;

  void publish_events(double_t time, const std::vector<Event> &events);

  void load_processor_script(std::string processor_type, std::string processor_script_path);
};


template<class SubscriptionType, class PublisherType>
ProcessorNode<SubscriptionType, PublisherType>::ProcessorNode(std::string node_name,
                                                              std::string subscription_topic,
                                                              std::string publisher_topic) : Node(node_name) {
  std::string processor_type;
  this->declare_parameter<std::string>("processor_type", "cpp");
  this->get_parameter("processor_type", processor_type);

  std::string processor_script_path;
  this->declare_parameter<std::string>("processor_script", "");
  this->get_parameter("processor_script", processor_script_path);

  this->load_processor_script(processor_type, processor_script_path);

  auto subscription_callback = [this](const std::shared_ptr<SubscriptionType> message) -> void {
    auto events = processor->data_received(*message);
    publish_events(message->time, events);
  };

  this->subscription = this->template create_subscription<SubscriptionType>(subscription_topic, 5000,
                                                                            subscription_callback);
  this->publisher = this->template create_publisher<PublisherType>(publisher_topic, 5000);

}

template<class SubscriptionType, class PublisherType>
void ProcessorNode<SubscriptionType, PublisherType>::publish_events(double_t time,
                                                                    const std::vector<Event> &events) {

  for (Event event: events) {
    PublisherType ros_event;

    ros_event.event_type = event.event_type;
    ros_event.processing_start_time = time;

    switch (event.event_type) {
      case PULSE:
        ros_event.when_to_execute = event.pulse.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published pulse event timed at %.4f.",
                    ros_event.when_to_execute);
        break;

      case CHARGE:
        ros_event.when_to_execute = event.charge.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published charge event timed at %.4f.",
                    ros_event.when_to_execute);
        break;

      case DISCHARGE:
        ros_event.when_to_execute = event.discharge.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published discharge event timed at %.4f.",
                    ros_event.when_to_execute);
        break;

      case SIGNAL_OUT:
        ros_event.when_to_execute = event.signal_out.event.time;
        RCLCPP_INFO(rclcpp::get_logger("eeg_processor"), "Published signal out event timed at %.4f.",
                    ros_event.when_to_execute);
        break;

      default:
        RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Warning, unknown fpga event type: %d", event.event_type);
    }

    publisher->publish(ros_event);

  }

}

template<class SubscriptionType, class PublisherType>
void ProcessorNode<SubscriptionType, PublisherType>::load_processor_script(std::string processor_type,
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
