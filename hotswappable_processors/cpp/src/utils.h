//
// Created by alqio on 14.11.2022.
//

#include <cstdint>
#include <vector>
#include "fpga_event.h"

#ifndef PROCESSOR_FACTORY_UTILS_H
#define PROCESSOR_FACTORY_UTILS_H


fpga_event create_charge_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double time, uint16_t target_voltage);

fpga_event create_discharge_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double time, uint16_t target_voltage);

fpga_event create_pulse_command(uint16_t event_id, uint8_t channel, uint8_t execution_condition, double time);

fpga_event create_signal_out_command(uint16_t event_id, uint8_t index, uint16_t duration_us, uint8_t execution_condition, double time);

std::vector<waveform_piece> get_default_pulse_waveform(uint8_t channel);

#endif //PROCESSOR_FACTORY_UTILS_H
