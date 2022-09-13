//
// Created by alqio on 1.9.2022.
//

#include "headers/cpp_processor.h"

CPPProcessor::CPPProcessor(const std::string &script_path) {
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

std::vector<FpgaEvent> CPPProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  coder::array<matlab_fpga_event, 1U> events;

  inner_processor->data_received(data.channel_datapoint.data(), events);

  std::vector<FpgaEvent> output;

  for (auto i = events.begin(); i != events.end(); i++) {
    auto event = *i;
    FpgaEvent fpga_event;
    if (event.event_type == CHARGE_EVENT) {
      fpga_event.event_type = CHARGE_EVENT;
      fpga_event.charge_event = fpga_interfaces::msg::ChargeEvent();
      fpga_event.charge_event.event_info.event_id = event.b_event_info.event_id;
      fpga_event.charge_event.event_info.execution_condition = event.b_event_info.execution_condition;
      fpga_event.charge_event.event_info.time_us = event.b_event_info.time_us;
      fpga_event.charge_event.channel = event.channel;
      fpga_event.charge_event.target_voltage = event.target_voltage;

    } else if (event.event_type == STIMULATION_PULSE_EVENT) {
      fpga_event.event_type = STIMULATION_PULSE_EVENT;
      fpga_event.stimulation_pulse_event = fpga_interfaces::msg::StimulationPulseEvent();

      for (auto piece: event.pieces) {
        auto fpga_piece = fpga_interfaces::msg::StimulationPulsePiece();
        fpga_piece.duration_in_ticks = piece.duration_in_ticks;
        fpga_piece.mode = piece.mode;
        fpga_event.stimulation_pulse_event.pieces.push_back(fpga_piece);
      }
      fpga_event.stimulation_pulse_event.event_info.event_id = event.b_event_info.event_id;
      fpga_event.stimulation_pulse_event.event_info.execution_condition = event.b_event_info.execution_condition;
      fpga_event.stimulation_pulse_event.event_info.time_us = event.b_event_info.time_us;
      fpga_event.stimulation_pulse_event.channel = event.channel;

    } else {
      fpga_event.event_type = DISCHARGE_EVENT;
      fpga_event.discharge_event = fpga_interfaces::msg::DischargeEvent();
      fpga_event.discharge_event.event_info.event_id = event.b_event_info.event_id;
      fpga_event.discharge_event.event_info.execution_condition = event.b_event_info.execution_condition;
      fpga_event.discharge_event.event_info.time_us = event.b_event_info.time_us;
      fpga_event.discharge_event.channel = event.channel;
      fpga_event.discharge_event.target_voltage = event.target_voltage;
    }
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

void CPPProcessor::init() {
  std::cout << "in CPPPprocessor init" << std::endl;
}

int CPPProcessor::close() {
  inner_processor->end_experiment();

  //Empty the pointer
  //inner_processor.reset();

  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
  return 0;
}
