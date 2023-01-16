//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_FACTORY_CPP_PROCESSOR_H
#define PROCESSOR_FACTORY_CPP_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include "cpp_processor_interface.h"
#include "matlab_helpers.h"


template<class InputType, class OutputType>
class CppProcessor : public ProcessorWrapper<InputType, OutputType> {
public:
  explicit CppProcessor(const std::string &script_path);

  ~CppProcessor();

  std::vector<OutputType> init() override;

  std::vector<OutputType> eeg_received(mtms_interfaces::msg::EegDatapoint sample) override;

  std::vector<OutputType> event_received(mtms_interfaces::msg::Event event) override;

private:
  void *processor_factory;
  std::unique_ptr<CppProcessorInterface> inner_processor;
};


template<class InputType, class OutputType>
CppProcessor<InputType, OutputType>::CppProcessor(const std::string &script_path) {
  processor_factory = dlopen(script_path.c_str(), RTLD_NOW);
  if (processor_factory == nullptr) {
    std::cerr << "Cannot load processor_factory: " << dlerror() << std::endl;
  }
  auto *create_processor_func = reinterpret_cast<create_cpp_processor >(dlsym(processor_factory, "create_processor"));

  if (!create_processor_func) {
    std::cerr << "Cannot load create_processor_func symbols: " << dlerror() << std::endl;
  }

  inner_processor = std::unique_ptr<CppProcessorInterface>(create_processor_func());
}

template<class InputType, class OutputType>
std::vector<OutputType> CppProcessor<InputType, OutputType>::event_received(mtms_interfaces::msg::Event event) {

}

template<class InputType, class OutputType>
std::vector<OutputType> CppProcessor<InputType, OutputType>::eeg_received(mtms_interfaces::msg::EegDatapoint data) {
  auto events = inner_processor->data_received(
      data.eeg_channels,
      data.time,
      data.first_sample_of_experiment
  );

  std::vector<Event> output;

  for (auto i = 0; i < events.size(); i++) {
    auto event = events[i];
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    output.push_back(fpga_event);
  }

  return output;
}

template<class InputType, class OutputType>
std::vector<OutputType> CppProcessor<InputType, OutputType>::init() {
  std::vector<Event> fpga_events;
  auto events = inner_processor->init_experiment();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    fpga_events.push_back(fpga_event);
  }
  return fpga_events;
}

template<class InputType, class OutputType>
CppProcessor<InputType, OutputType>::~CppProcessor() {
  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
}


#endif //PROCESSOR_FACTORY_CPP_PROCESSOR_H
