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

  std::vector<OutputType> init() override;

  std::vector<OutputType> eeg_received(mtms_interfaces::msg::EegDatapoint sample) override;

  std::vector<OutputType> event_received(mtms_interfaces::msg::Event event) override;

  void close();

private:
  void *processor_factory;
  std::unique_ptr<MatlabProcessorInterface> inner_processor;
};


#endif //EEG_PROCESSOR_CPP_PROCESSOR_H
