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

std::vector<Event> CppProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  auto events = inner_processor->data_received(
      data.eeg_channels,
      data.time,
      data.first_sample_of_experiment
  );

  std::vector<Event> events_out;

  for (auto i = 0; i < events.size(); i++) {
    auto matlab_event = events[i];
    auto event = convert_matlab_event_to_event(matlab_event);
    events_out.push_back(event);
  }

  return events_out;
}

std::vector<Event> CppProcessor::init() {
  std::vector<Event> events_out;

  auto events = inner_processor->init_experiment();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    events_out.push_back(event);
  }

  return events_out;
}

std::vector<Event> CppProcessor::close() {
  std::vector<Event> events_out;

  auto events = inner_processor->end_experiment();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    events_out.push_back(event);
  }

  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
  return events_out;
}
