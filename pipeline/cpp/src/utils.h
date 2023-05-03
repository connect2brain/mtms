//
// Created by alqio on 14.11.2022.
//

#include <cstdint>
#include <vector>
#include "mtms_device_event.h"

#ifndef PROCESSOR_FACTORY_UTILS_H
#define PROCESSOR_FACTORY_UTILS_H


mtms_device_event create_charge_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double execution_time, uint16_t target_voltage);

mtms_device_event create_discharge_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double execution_time, uint16_t target_voltage);

mtms_device_event create_pulse_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double execution_time);

mtms_device_event create_trigger_out_command(uint16_t event_id, uint8_t index, uint16_t duration_us, uint8_t execution_condition, double execution_time);

mtms_device_event create_stimulus_command(uint16_t event_id, uint16_t state, uint8_t execution_condition, double execution_time);

std::vector<waveform_piece> get_default_pulse_waveform(uint8_t channel);

#endif //PROCESSOR_FACTORY_UTILS_H
