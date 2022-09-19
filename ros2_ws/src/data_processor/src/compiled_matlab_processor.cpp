//
// Created by alqio on 1.9.2022.
//

#include "headers/compiled_matlab_processor.h"

CompiledMatlabProcessor::CompiledMatlabProcessor(const std::string &script_path) {
  processor_factory = dlopen(script_path.c_str(), RTLD_NOW);
  if (processor_factory == nullptr) {
    std::cerr << "Cannot load processor_factory: " << dlerror() << std::endl;
  }
  auto *create_processor_func = reinterpret_cast<create_processor >(dlsym(processor_factory, "create_processor"));

  if (!create_processor_func) {
    std::cerr << "Cannot load create_processor_func symbols: " << dlerror() << std::endl;
  }

  inner_processor = std::unique_ptr<MatlabProcessorInterface>(create_processor_func(50, 62));

}

std::vector<FpgaEvent> CompiledMatlabProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  coder::array<matlab_fpga_event, 1U> events;

  inner_processor->data_received(data.channel_datapoint.data(), events);

  std::vector<FpgaEvent> output;

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    auto fpga_event = convert_matlab_fpga_event_to_fpga_event(event);
    output.push_back(fpga_event);
  }
  /*
  if (!output.empty()) {
    std::cout << "Received " << output.size() << " events" << std::endl;
    for (const auto& event: output) {
      std::cout << "event type: " << event.to_string() << std::endl;
    }
  }
*/
  return output;
}

void CompiledMatlabProcessor::init() {
  std::cout << "in CPPPprocessor init" << std::endl;
}

int CompiledMatlabProcessor::close() {
  inner_processor->end_experiment();

  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
  return 0;
}
