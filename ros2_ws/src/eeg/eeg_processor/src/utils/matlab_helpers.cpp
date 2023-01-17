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
  std::cout << "  Execution time: " << event.b_event_info.execution_time << std::endl;
  std::cout << "Waveform: " << std::endl;
  for (auto i = 0; i < 3; i++) {
    std::cout << "  Phase: " << +event.waveform[i].waveform_phase << ", duration in ticks: "
              << event.waveform[i].duration_in_ticks
              << std::endl;
  }
}

eeg_interfaces::msg::EegDatapoint convert_matlab_sample_to_sample(matlab_eeg_sample matlab_sample) {
  eeg_interfaces::msg::EegDatapoint sample;
  sample.first_sample_of_experiment = matlab_sample.first_sample_of_experiment;
  sample.eeg_channels = matlab_sample.channel_data;
  sample.time = matlab_sample.time;
  return sample;
}

Event convert_matlab_event_to_event(matlab_event event) {
  Event matlab_event;
  if (event.event_type == CHARGE) {
    matlab_event.event_type = CHARGE;
    matlab_event.charge = event_interfaces::msg::Charge();
    matlab_event.charge.event_info.id = event.b_event_info.id;
    matlab_event.charge.event_info.execution_condition.value = event.b_event_info.execution_condition;
    matlab_event.charge.event_info.execution_time = event.b_event_info.execution_time;
    matlab_event.charge.channel = event.channel;
    matlab_event.charge.target_voltage = event.target_voltage;

  } else if (event.event_type == PULSE) {
    matlab_event.event_type = PULSE;
    matlab_event.pulse = event_interfaces::msg::Pulse();

    for (auto piece: event.waveform) {
      auto fpga_piece = event_interfaces::msg::WaveformPiece();
      fpga_piece.duration_in_ticks = piece.duration_in_ticks;
      fpga_piece.waveform_phase.value = piece.waveform_phase;
      matlab_event.pulse.waveform.push_back(fpga_piece);
    }
    matlab_event.pulse.event_info.id = event.b_event_info.id;
    matlab_event.pulse.event_info.execution_condition.value = event.b_event_info.execution_condition;
    matlab_event.pulse.event_info.execution_time = event.b_event_info.execution_time;
    matlab_event.pulse.channel = event.channel;

  } else if (event.event_type == DISCHARGE) {
    matlab_event.event_type = DISCHARGE;
    matlab_event.discharge = event_interfaces::msg::Discharge();
    matlab_event.discharge.event_info.id = event.b_event_info.id;
    matlab_event.discharge.event_info.execution_condition.value = event.b_event_info.execution_condition;
    matlab_event.discharge.event_info.execution_time = event.b_event_info.execution_time;
    matlab_event.discharge.channel = event.channel;
    matlab_event.discharge.target_voltage = event.target_voltage;
  } else {
    matlab_event.event_type = SIGNAL_OUT;
    matlab_event.signal_out = event_interfaces::msg::SignalOut();
    matlab_event.signal_out.event_info.id = event.b_event_info.id;
    matlab_event.signal_out.event_info.execution_condition.value = event.b_event_info.execution_condition;
    matlab_event.signal_out.event_info.execution_time = event.b_event_info.execution_time;
    matlab_event.signal_out.port = event.channel;
    matlab_event.signal_out.duration_us = event.duration_us;
  }
  return matlab_event;
}
