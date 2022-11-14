//
// Created by alqio on 14.9.2022.
//

#include "matlab_helpers.h"

void print_matlab_fpga_event(matlab_fpga_event event) {
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
  std::cout << "Event: " << std::endl;
  std::cout << "  Id: " << event.b_event.id << std::endl;
  std::cout << "  Execution condition: " << +event.b_event.execution_condition << std::endl;
  std::cout << "  Time: " << event.b_event.time << std::endl;
  std::cout << "Waveform: " << std::endl;
  for (auto i = 0; i < 3; i++) {
    std::cout << "  Phase: " << +event.waveform[i].waveform_phase << ", duration in ticks: " << event.waveform[i].duration_in_ticks
              << std::endl;
  }
}

FpgaEvent convert_matlab_fpga_event_to_fpga_event(matlab_fpga_event event) {
  FpgaEvent fpga_event;
  if (event.event_type == CHARGE) {
    fpga_event.event_type = CHARGE;
    fpga_event.charge = fpga_interfaces::msg::Charge();
    fpga_event.charge.event.id = event.b_event.id;
    fpga_event.charge.event.execution_condition.value = event.b_event.execution_condition;
    fpga_event.charge.event.time = event.b_event.time;
    fpga_event.charge.channel = event.channel;
    fpga_event.charge.target_voltage = event.target_voltage;

  } else if (event.event_type == PULSE) {
    fpga_event.event_type = PULSE;
    fpga_event.pulse = fpga_interfaces::msg::Pulse();

    for (auto piece: event.waveform) {
      auto fpga_piece = fpga_interfaces::msg::WaveformPiece();
      fpga_piece.duration_in_ticks = piece.duration_in_ticks;
      fpga_piece.waveform_phase.value = piece.waveform_phase;
      fpga_event.pulse.waveform.push_back(fpga_piece);
    }
    fpga_event.pulse.event.id = event.b_event.id;
    fpga_event.pulse.event.execution_condition.value = event.b_event.execution_condition;
    fpga_event.pulse.event.time = event.b_event.time;
    fpga_event.pulse.channel = event.channel;

  } else if (event.event_type == DISCHARGE) {
    fpga_event.event_type = DISCHARGE;
    fpga_event.discharge = fpga_interfaces::msg::Discharge();
    fpga_event.discharge.event.id = event.b_event.id;
    fpga_event.discharge.event.execution_condition.value = event.b_event.execution_condition;
    fpga_event.discharge.event.time = event.b_event.time;
    fpga_event.discharge.channel = event.channel;
    fpga_event.discharge.target_voltage = event.target_voltage;
  } else {
    fpga_event.event_type = SIGNAL_OUT;
    fpga_event.signal_out = fpga_interfaces::msg::SignalOut();
    fpga_event.signal_out.event.id = event.b_event.id;
    fpga_event.signal_out.event.execution_condition.value = event.b_event.execution_condition;
    fpga_event.signal_out.event.time = event.b_event.time;
    fpga_event.signal_out.port = event.channel;
  }
  return fpga_event;
}
