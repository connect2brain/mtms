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


template<class SubscriptionType, class InputType, class OutputType>
class ProcessorNode : public rclcpp::Node {
public:
  ProcessorNode(std::string node_name);

  typename rclcpp::Subscription<SubscriptionType>::SharedPtr subscription;

  ProcessorWrapper<InputType, OutputType> *processor;

  virtual void publish_events(double_t time, const std::vector<OutputType> &events) = 0;

  void load_processor_script(std::string processor_type, std::string processor_script_path);

};


template<class SubscriptionType, class InputType, class OutputType>
ProcessorNode<SubscriptionType, InputType, OutputType>::ProcessorNode(std::string node_name) : Node(
    node_name) {
  std::string processor_type;
  this->declare_parameter<std::string>("processor_type", "cpp");
  this->get_parameter("processor_type", processor_type);

  std::string processor_script_path;
  this->declare_parameter<std::string>("processor_script", "");
  this->get_parameter("processor_script", processor_script_path);

  this->load_processor_script(processor_type, processor_script_path);


}

template<class SubscriptionType, class InputType, class OutputType>
void ProcessorNode<SubscriptionType, InputType, OutputType>::load_processor_script(std::string processor_type,
                                                                                   std::string processor_script_path) {
  if (processor_type == "python") {
#ifdef PYTHON_FOUND
    processor = new PythonProcessor<InputType, OutputType>(processor_script_path);
#else
    RCLCPP_ERROR(this->get_logger(), "ERROR: Trying to use python processor but compiled without python!");
#endif

  } else if (processor_type == "matlab") {
#ifdef MATLAB_FOUND
    processor = new MatlabProcessor<InputType, OutputType>(processor_script_path);
#else
    RCLCPP_ERROR(this->get_logger(), "ERROR: Trying to use MATLAB processor but compiled without MATLAB!");
#endif

  } else if (processor_type == "compiledmatlab") {
    processor = new CompiledMatlabProcessor<InputType, OutputType>(processor_script_path);

  } else if (processor_type == "cpp") {
    processor = new CppProcessor<InputType, OutputType>(processor_script_path);

  } else {
    RCLCPP_ERROR(this->get_logger(), "No processor specified!");
  }
}


#endif //EEG_PROCESSOR_PROCESSOR_NODE_H
