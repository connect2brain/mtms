//
// Created by alqio on 14.9.2022.
//

#include "headers/matlab_helpers.h"

void print_matlab_fpga_event(matlab_fpga_event event) {
  std::cout << "Event type: ";
  if (event.event_type == STIMULATION_PULSE_EVENT) {
    std::cout << "Stimulation pulse event" << std::endl;
  } else if (event.event_type == CHARGE_EVENT) {
    std::cout << "Charge event" << std::endl;
  } else if (event.event_type == DISCHARGE_EVENT) {
    std::cout << "Discharge event" << std::endl;
  } else {
    std::cout << "Uknown event type " << +event.event_type << std::endl;
  }
  std::cout << "Channel: " << +event.channel << std::endl;
  std::cout << "Target voltage: " << event.target_voltage << std::endl;
  std::cout << "Event info: " << std::endl;
  std::cout << "  Event id: " << event.b_event_info.event_id << std::endl;
  std::cout << "  Execution condition: " << +event.b_event_info.execution_condition << std::endl;
  std::cout << "  Time us: " << event.b_event_info.time_us << std::endl;
  std::cout << "Pieces: " << std::endl;
  for (auto i = 0; i < 3; i++) {
    std::cout << "  mode: " << +event.pieces[i].mode << ", duration in ticks: " << event.pieces[i].duration_in_ticks
              << std::endl;
  }
}

FpgaEvent convert_matlab_fpga_event_to_fpga_event(matlab_fpga_event event) {
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
  return fpga_event;
}