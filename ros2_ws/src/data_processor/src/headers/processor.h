//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_PROCESSOR_H
#define DATA_PROCESSOR_PROCESSOR_H

#include "string"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/charge_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"

enum FpgaEventType {
  STIMULATION_PULSE_EVENT,
  CHARGE_EVENT
};

struct FpgaEvent {
  fpga_interfaces::msg::StimulationPulseEvent stimulation_pulse_event;
  fpga_interfaces::msg::ChargeEvent charge_event;
  FpgaEventType event_type;

  template<typename T>
  T event() {
    if (event_type == STIMULATION_PULSE_EVENT) {
      return stimulation_pulse_event;
    } else if (event_type == CHARGE_EVENT) {
      return charge_event;
    }
  }

};

class ProcessorWrapper {
public:
  virtual void init() = 0;

  virtual std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data) = 0;

  virtual int close() = 0;

};

#endif //DATA_PROCESSOR_PROCESSOR_H
