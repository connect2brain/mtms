//
// Created by alqio on 9/12/22.
//

#ifndef DATA_PROCESSOR_FPGA_EVENT_H
#define DATA_PROCESSOR_FPGA_EVENT_H

#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/charge_event.hpp"
#include "fpga_interfaces/msg/discharge_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"

enum FpgaEventType {
  STIMULATION_PULSE_EVENT = 0,
  CHARGE_EVENT = 1,
  DISCHARGE_EVENT = 2
};

struct FpgaEvent {
  fpga_interfaces::msg::StimulationPulseEvent stimulation_pulse_event;
  fpga_interfaces::msg::ChargeEvent charge_event;
  fpga_interfaces::msg::DischargeEvent discharge_event;
  FpgaEventType event_type = STIMULATION_PULSE_EVENT;

  template<typename T>
  T event() {
    if (event_type == STIMULATION_PULSE_EVENT) {
      return stimulation_pulse_event;
    } else if (event_type == CHARGE_EVENT) {
      return charge_event;
    } else {
      return discharge_event;
    }
  }

  [[nodiscard]] std::string to_string() const {
    if (event_type == STIMULATION_PULSE_EVENT) {
      return "Stimulation pulse event";
    } else if (event_type == CHARGE_EVENT) {
      return "Charge event";
    } else {
      return "Discharge event";
    }
  }

};

#endif //DATA_PROCESSOR_FPGA_EVENT_H
