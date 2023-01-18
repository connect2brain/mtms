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

class CompiledMatlabProcessor : public ProcessorWrapper {
public:
  explicit CompiledMatlabProcessor(const std::string &script_path);

  ~CompiledMatlabProcessor();

  std::vector<Event> init();

  std::vector<eeg_interfaces::msg::EegDatapoint> raw_eeg_received(eeg_interfaces::msg::EegDatapoint sample);

  std::vector<Event> cleaned_eeg_received(eeg_interfaces::msg::EegDatapoint sample);

  std::vector<Event> present_stimulus_received(event_interfaces::msg::Stimulus event);

private:
  void *processor_factory;
  std::unique_ptr<MatlabProcessorInterface> inner_processor;
};


#endif //EEG_PROCESSOR_CPP_PROCESSOR_H
