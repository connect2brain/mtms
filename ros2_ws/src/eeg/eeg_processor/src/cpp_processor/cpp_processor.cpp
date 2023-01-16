//
// Created by alqio on 1.9.2022.
//

#include "cpp_processor.h"
#include "matlab_helpers.h"

CppProcessor::CppProcessor(const std::string &script_path) {
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

std::vector<Event> CppProcessor::present_stimulus_received(mtms_interfaces::msg::Event event) {}


std::vector<mtms_interfaces::msg::EegDatapoint> CppProcessor::raw_eeg_received(mtms_interfaces::msg::EegDatapoint sample) {
}

std::vector<Event> CppProcessor::cleaned_eeg_received(mtms_interfaces::msg::EegDatapoint sample) {
  auto events = inner_processor->data_received(
      sample.eeg_channels,
      sample.time,
      sample.first_sample_of_experiment
  );

  std::vector<Event> output;

  for (auto i = 0; i < events.size(); i++) {
    auto event = events[i];
    auto matlab_event = convert_matlab_event_to_event(event);
    output.push_back(matlab_event);
  }

  return output;
}

std::vector<Event> CppProcessor::init() {
  std::vector<Event> matlab_events;
  auto events = inner_processor->init_experiment();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    auto fpga_event = convert_matlab_event_to_event(event);
    inner_processor.push_back(fpga_event);
  }
  return inner_processor;
}

CppProcessor::~CppProcessor() {
  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
}