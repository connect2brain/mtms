//
// Created by alqio on 14.9.2022.
//

#include "matlab_helpers.h"

void print_matlab_event(matlab_event event) {
  std::cout << "Event type: ";
  if (event.event_type == PULSE) {
    std::cout << "Pulse" << std::endl;
  } else if (event.event_type == CHARGE) {
    std::cout << "Charge" << std::endl;
  } else if (event.event_type == DISCHARGE) {
    std::cout << "Discharge" << std::endl;
  } else {
    std::cout << "Unknown event type " << +event.event_type << std::endl;
  }
  std::cout << "Channel: " << +event.channel << std::endl;
  std::cout << "Target voltage: " << event.target_voltage << std::endl;
  std::cout << "Event info: " << std::endl;
  std::cout << "  Id: " << event.b_event_info.id << std::endl;
  std::cout << "  Execution condition: " << +event.b_event_info.execution_condition << std::endl;
  std::cout << "  Time: " << event.b_event_info.time << std::endl;
  std::cout << "Waveform: " << std::endl;
  for (auto i = 0; i < 3; i++) {
    std::cout << "  Phase: " << +event.waveform[i].waveform_phase << ", duration in ticks: " << event.waveform[i].duration_in_ticks
              << std::endl;
  }
}

Event convert_matlab_event_to_event(matlab_event event) {
  Event fpga_event;

  if (event.event_type == CHARGE) {
    fpga_event.event_type = CHARGE;
    fpga_event.charge = event_interfaces::msg::Charge();
    fpga_event.charge.event_info.id = event.b_event_info.id;
    fpga_event.charge.event_info.execution_condition.value = event.b_event_info.execution_condition;
    fpga_event.charge.event_info.time = event.b_event_info.time;
    fpga_event.charge.channel = event.channel;
    fpga_event.charge.target_voltage = event.target_voltage;

  } else if (event.event_type == PULSE) {
    fpga_event.event_type = PULSE;
    fpga_event.pulse = event_interfaces::msg::Pulse();

    for (auto piece: event.waveform) {
      auto fpga_piece = event_interfaces::msg::WaveformPiece();
      fpga_piece.duration_in_ticks = piece.duration_in_ticks;
      fpga_piece.waveform_phase.value = piece.waveform_phase;
      fpga_event.pulse.waveform.push_back(fpga_piece);
    }
    fpga_event.pulse.event_info.id = event.b_event_info.id;
    fpga_event.pulse.event_info.execution_condition.value = event.b_event_info.execution_condition;
    fpga_event.pulse.event_info.time = event.b_event_info.time;
    fpga_event.pulse.channel = event.channel;

  } else if (event.event_type == DISCHARGE) {
    fpga_event.event_type = DISCHARGE;
    fpga_event.discharge = event_interfaces::msg::Discharge();
    fpga_event.discharge.event_info.id = event.b_event_info.id;
    fpga_event.discharge.event_info.execution_condition.value = event.b_event_info.execution_condition;
    fpga_event.discharge.event_info.time = event.b_event_info.time;
    fpga_event.discharge.channel = event.channel;
    fpga_event.discharge.target_voltage = event.target_voltage;

  } else {
    fpga_event.event_type = SIGNAL_OUT;
    fpga_event.signal_out = event_interfaces::msg::SignalOut();
    fpga_event.signal_out.event_info.id = event.b_event_info.id;
    fpga_event.signal_out.event_info.execution_condition.value = event.b_event_info.execution_condition;
    fpga_event.signal_out.event_info.time = event.b_event_info.time;
    fpga_event.signal_out.port = event.channel;
    fpga_event.signal_out.duration_us = event.duration_us;
  }
  return fpga_event;
}
