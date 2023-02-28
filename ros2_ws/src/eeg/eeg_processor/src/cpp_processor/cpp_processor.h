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

class CppProcessor : public ProcessorWrapper {
public:
  explicit CppProcessor(const std::string &script_path);

  ~CppProcessor();

  std::vector<Event> init();

  std::vector<eeg_interfaces::msg::EegDatapoint> raw_eeg_received(eeg_interfaces::msg::EegDatapoint sample);

  std::vector<Event> cleaned_eeg_received(eeg_interfaces::msg::EegDatapoint sample);

  std::vector<Event> present_stimulus_received(event_interfaces::msg::Stimulus event);

  std::vector<Event> end_experiment();

private:
  void *processor_factory;
  std::unique_ptr<CppProcessorInterface> inner_processor;
};


#endif //PROCESSOR_FACTORY_CPP_PROCESSOR_H
