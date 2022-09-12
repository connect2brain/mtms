//
// Created by alqio on 1.9.2022.
//

#include "headers/cpp_processor.h"

double ffRand(double fMin, double fMax) {
  double f = (double) rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

CPPProcessor::CPPProcessor(const std::string &script_path) {
  processor_factory = dlopen(script_path.c_str(), RTLD_NOW);
  if (processor_factory == nullptr) {
    std::cerr << "Cannot load processor_factory: " << dlerror() << std::endl;
  }
  std::cout << "loaded factory" << std::endl;
  auto *create_processor_func = reinterpret_cast<create_processor >(dlsym(processor_factory, "create_processor"));
  auto *destroy_processor_func = reinterpret_cast<destroy_processor >(dlsym(processor_factory, "destroy_processor"));

  if (!create_processor_func) {
    std::cerr << "Cannot load create_processor_func symbols: " << dlerror() << std::endl;
  }
  if (!destroy_processor_func) {
    std::cerr << "Cannot load destroy_processor_func symbols: " << dlerror() << std::endl;
  }

  inner_processor = std::unique_ptr<MatlabProcessorInterface>(create_processor_func(50, 62));

}

std::vector<FpgaEvent> CPPProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  coder::array<stimulation_pulse_event, 1U> pulses;
  inner_processor->data_received(data.channel_datapoint.data(), pulses);

  std::vector<FpgaEvent> output;
  for (auto i = pulses.begin(); i != pulses.end(); i++) {
    auto event = *i;
    FpgaEvent fpga_event;
    fpga_event.event_type = STIMULATION_PULSE_EVENT;
    fpga_event.stimulation_pulse_event = fpga_interfaces::msg::StimulationPulseEvent();

    fpga_event.stimulation_pulse_event.channel = event.channel;

    fpga_event.stimulation_pulse_event.event_info.event_id = event.b_event_info.event_id;
    fpga_event.stimulation_pulse_event.event_info.execution_condition = event.b_event_info.wait_for_trigger;
    fpga_event.stimulation_pulse_event.event_info.time_us = event.b_event_info.time_us;

    for (auto piece: event.pieces) {
      auto fpga_piece = fpga_interfaces::msg::StimulationPulsePiece();
      fpga_piece.duration_in_ticks = piece.duration_in_ticks;
      fpga_piece.mode = piece.mode;
      fpga_event.stimulation_pulse_event.pieces.push_back(fpga_piece);
    }
    output.push_back(fpga_event);
  }
  if (!output.empty()) {
    std::cout << "Pulse count: " << output.size() << std::endl;
  }

  return output;
}

void CPPProcessor::init() {
  std::cout << "in CPPPprocessor init" << std::endl;
}

int CPPProcessor::close() {
  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
  return 0;
}
