  //
// Created by alqio on 14.11.2022.
//

#include "utils.h"

mtms_device_event create_pulse_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double execution_time) {
  mtms_device_event event;
  event.channel = channel;
  event.event_type = 0;

  event.b_event_info.id = event_id;
  event.b_event_info.execution_time = execution_time;
  event.b_event_info.execution_condition = execution_condition;

  auto default_waveform = get_default_pulse_waveform(channel);
  for (auto i = 0; i < 3; i++) {
    event.waveform[i] = default_waveform[i];
  }

  return event;
}


mtms_device_event create_charge_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double execution_time,
                                 uint16_t target_voltage) {
  mtms_device_event event;
  event.channel = channel;
  event.event_type = 1;

  event.b_event_info.id = event_id;
  event.b_event_info.execution_time = execution_time;
  event.b_event_info.execution_condition = execution_condition;

  event.target_voltage = target_voltage;
  return event;
}

mtms_device_event create_stimulus_command(uint16_t event_id, uint16_t state, uint8_t execution_condition, double execution_time) {
  mtms_device_event event;
  event.event_type = 4;
  event.state = state;
  event.b_event_info.id = event_id;
  event.b_event_info.execution_time = execution_time;
  event.b_event_info.execution_condition = execution_condition;

  return event;
}

mtms_device_event create_discharge_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double execution_time,
                                    uint16_t target_voltage) {
  mtms_device_event event;
  event.channel = channel;
  event.event_type = 2;

  event.b_event_info.id = event_id;
  event.b_event_info.execution_time = execution_time;
  event.b_event_info.execution_condition = execution_condition;

  event.target_voltage = target_voltage;
  return event;
}


mtms_device_event create_signal_out_command(uint16_t event_id, uint8_t index, uint16_t duration_us, uint8_t execution_condition, double execution_time) {
  mtms_device_event event;
  event.channel = index;
  event.event_type = 3;
  event.duration_us = duration_us;

  event.b_event_info.id = event_id;
  event.b_event_info.execution_time = execution_time;
  event.b_event_info.execution_condition = execution_condition;

  return event;
}

std::vector<waveform_piece> get_default_pulse_waveform(uint8_t channel) {
  std::vector<waveform_piece> pieces;

  waveform_piece piece1;
  waveform_piece piece2;
  waveform_piece piece3;

  piece1.waveform_phase = 0;
  piece1.duration_in_ticks = 2400;

  piece2.waveform_phase = 1;
  piece2.duration_in_ticks = 1200;

  piece3.waveform_phase = 2;
  if (channel == 1 || channel == 2) {
    piece3.duration_in_ticks = 1480;
  } else if (channel == 3 || channel == 4) {
    piece3.duration_in_ticks = 1564;
  } else {
    piece3.duration_in_ticks = 1776;
  }

  pieces.push_back(piece1);
  pieces.push_back(piece2);
  pieces.push_back(piece3);
  return pieces;
}
