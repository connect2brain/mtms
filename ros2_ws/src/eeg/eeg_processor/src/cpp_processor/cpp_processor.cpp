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

std::vector<Event> CppProcessor::present_stimulus_received(event_interfaces::msg::Stimulus event) {}


std::vector<eeg_interfaces::msg::EegSample> CppProcessor::raw_eeg_received(eeg_interfaces::msg::EegSample sample) {
  auto samples = inner_processor->raw_eeg_received(
      sample.eeg_channels,
      sample.time,
      sample.first_sample_of_session
  );

  std::vector<eeg_interfaces::msg::EegSample> cleaned_samples;

  for (unsigned long i = 0; i < samples.size(); i++) {
    auto matlab_eeg_sample = samples[i];
    eeg_interfaces::msg::EegSample new_sample;
    new_sample.eeg_channels = matlab_eeg_sample.channel_data;
    new_sample.time = matlab_eeg_sample.time;
    new_sample.first_sample_of_session = matlab_eeg_sample.first_sample_of_session;
    cleaned_samples.push_back(new_sample);
  }

  return cleaned_samples;
}

std::vector<Event> CppProcessor::cleaned_eeg_received(eeg_interfaces::msg::EegSample sample) {
  auto events = inner_processor->data_received(
      sample.eeg_channels,
      sample.time,
      sample.first_sample_of_session
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
  std::vector<Event> matlab_events;
  auto events = inner_processor->init_session();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    matlab_events.push_back(event);
  }
  return matlab_events;
}

std::vector<Event> CppProcessor::end_session() {
  std::vector<Event> matlab_events;
  auto events = inner_processor->end_session();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    matlab_events.push_back(event);
  }
  return matlab_events;
}

CppProcessor::~CppProcessor() {
  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
}
