//
// Created by alqio on 1.9.2022.
//

#include "compiled_matlab_processor.h"

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
CompiledMatlabProcessor<InputType, OutputType>::eeg_received(mtms_interfaces::msg::EegDatapoint sample) {
  coder::array<matlab_event, 1U> events;

  inner_processor->data_received(
      sample.eeg_channels.data(),
      sample.eeg_channels.size(),
      sample.time,
      sample.first_sample_of_experiment,
      events
  );

  std::vector<Event> events_out;

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    events_out.push_back(event);
  }

  return events_out;
}


template<class InputType, class OutputType>
std::vector<OutputType> CompiledMatlabProcessor<InputType, OutputType>::init() {
  coder::array<matlab_event, 1U> events;
  std::vector<Event> events_out;
  inner_processor->init_experiment(events);

  for (auto i = events.begin(); i != events.end(); i++) {
    auto matlab_event = *i;
    auto event = convert_matlab_event_to_event(matlab_event);
    events_out.push_back(event);
  }
  return events_out;
}

template<class InputType, class OutputType>
void CompiledMatlabProcessor<InputType, OutputType>::close() {

  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
}
