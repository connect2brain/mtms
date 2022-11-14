//
// Created by alqio on 1.9.2022.
//

#include "headers/cpp_processor.h"
#include "headers/matlab_helpers.h"

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

std::vector<FpgaEvent> CppProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  auto events = inner_processor->data_received(
      data.eeg_channels,
      data.time,
      data.first_sample_of_experiment
  );

  std::vector<FpgaEvent> output;

  for (auto i = 0; i < events.size(); i++) {
    auto event = events[i];
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    output.push_back(fpga_event);
  }

  return output;
}

std::vector<FpgaEvent> CppProcessor::init() {
  std::vector<FpgaEvent> fpga_events;
  auto events = inner_processor->init_experiment();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    fpga_events.push_back(fpga_event);
  }
  return fpga_events;
}

std::vector<FpgaEvent> CppProcessor::close() {
  std::vector<FpgaEvent> fpga_events;
  auto events = inner_processor->end_experiment();

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    fpga_events.push_back(fpga_event);
  }

  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
  return fpga_events;
}
