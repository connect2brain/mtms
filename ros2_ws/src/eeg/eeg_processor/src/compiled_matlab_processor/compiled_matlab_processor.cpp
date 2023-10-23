//
// Created by alqio on 1.9.2022.
//

#include "compiled_matlab_processor.h"

CompiledMatlabProcessor::CompiledMatlabProcessor(const std::string &script_path) {
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

std::vector<Event> CompiledMatlabProcessor::present_stimulus_received(event_interfaces::msg::Stimulus event) {}


std::vector<eeg_interfaces::msg::EegSample>
CompiledMatlabProcessor::raw_eeg_received(eeg_interfaces::msg::EegSample sample) {
  coder::array<matlab_event, 1U> events;
  coder::array<matlab_eeg_sample, 1U> samples;

  inner_processor->data_received(
      sample.eeg_channels.data(),
      sample.eeg_channels.size(),
      sample.time,
      sample.first_sample_of_session,
      events,
      samples
  );

  std::vector<eeg_interfaces::msg::EegSample> output;

  for (auto i = samples.begin(); i != samples.end(); i++) {
    auto matlab_sample = *i;
    auto new_sample = convert_matlab_sample_to_sample(matlab_sample);
    output.push_back(new_sample);
  }

  return output;

}

std::vector<Event> CompiledMatlabProcessor::cleaned_eeg_received(eeg_interfaces::msg::EegSample sample) {
  coder::array<matlab_event, 1U> events;
  coder::array<matlab_eeg_sample, 1U> samples;

  inner_processor->data_received(
      sample.eeg_channels.data(),
      sample.eeg_channels.size(),
      sample.time,
      sample.first_sample_of_session,
      events,
      samples
  );

  std::vector<Event> output;

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    output.push_back(event);
  }

  return output;
}

std::vector<Event> CompiledMatlabProcessor::init() {
  std::vector<Event> matlab_events;
  coder::array<matlab_event, 1U> events;
  inner_processor->init_session(events);

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    matlab_events.push_back(event);
  }
  return matlab_events;
}

std::vector<Event> CompiledMatlabProcessor::end_session() {
  std::vector<Event> matlab_events;
  coder::array<matlab_event, 1U> events;
  inner_processor->end_session(events);

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    matlab_events.push_back(event);
  }
  return matlab_events;
}

CompiledMatlabProcessor::~CompiledMatlabProcessor() {
  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
}
