//
// Created by alqio on 1.9.2022.
//

#ifndef EEG_PROCESSOR_CPP_PROCESSOR_H
#define EEG_PROCESSOR_CPP_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include "matlab_processor_interface.h"
#include "matlab_helpers.h"

template<class InputType, class OutputType>
class CompiledMatlabProcessor : public ProcessorWrapper<InputType, OutputType> {
public:
  explicit CompiledMatlabProcessor(const std::string &script_path);

  ~CompiledMatlabProcessor();

  std::vector<OutputType> init() override;

  std::vector<OutputType> eeg_received(mtms_interfaces::msg::EegDatapoint sample) override;

  std::vector<OutputType> event_received(mtms_interfaces::msg::Event event) override;

private:
  void *processor_factory;
  std::unique_ptr<MatlabProcessorInterface> inner_processor;
};

template<class InputType, class OutputType>
CompiledMatlabProcessor<InputType, OutputType>::CompiledMatlabProcessor(const std::string &script_path) {
  processor_factory = dlopen(script_path.c_str(), RTLD_NOW);
  if (processor_factory == nullptr) {
    std::cerr << "Cannot load processor_factory: " << dlerror() << std::endl;
  }
  auto *create_processor_func = reinterpret_cast<create_processor >(dlsym(processor_factory, "create_processor"));

  if (!create_processor_func) {
    std::cerr << "Cannot load create_processor_func symbols: " << dlerror() << std::endl;
  }

  inner_processor = std::unique_ptr<MatlabProcessorInterface>(create_processor_func());

}

template<class InputType, class OutputType>
std::vector<OutputType>
CompiledMatlabProcessor<InputType, OutputType>::event_received(mtms_interfaces::msg::Event event) {}

template<class InputType, class OutputType>
std::vector<OutputType>
CompiledMatlabProcessor<InputType, OutputType>::eeg_received(mtms_interfaces::msg::EegDatapoint sample) {
  coder::array<matlab_fpga_event, 1U> events;

  inner_processor->data_received(
      sample.eeg_channels.data(),
      sample.eeg_channels.size(),
      sample.time,
      sample.first_sample_of_experiment,
      events
  );

  std::vector<Event> output;

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    output.push_back(fpga_event);
  }

  return output;
}

template<class InputType, class OutputType>
std::vector<OutputType> CompiledMatlabProcessor<InputType, OutputType>::init() {
  std::vector<Event> fpga_events;
  coder::array<matlab_fpga_event, 1U> events;
  inner_processor->init_experiment(events);

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    fpga_events.push_back(fpga_event);
  }
  return fpga_events;
}

template<class InputType, class OutputType>
CompiledMatlabProcessor<InputType, OutputType>::~CompiledMatlabProcessor() {

  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
}


#endif //EEG_PROCESSOR_CPP_PROCESSOR_H
