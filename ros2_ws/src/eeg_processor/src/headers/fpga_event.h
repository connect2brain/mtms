//
// Created by alqio on 9/12/22.
//

#ifndef EEG_PROCESSOR_FPGA_EVENT_H
#define EEG_PROCESSOR_FPGA_EVENT_H

#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/charge_event.hpp"
#include "fpga_interfaces/msg/discharge_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"
#include <iostream>

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

  [[nodiscard]] std::string to_string() const {
    if (event_type == STIMULATION_PULSE_EVENT) {
      return "Stimulation pulse event";
    } else if (event_type == CHARGE_EVENT) {
      return "Charge event";
    } else {
      return "Discharge event";
    }
  }

  void print() {
    std::cout << "Event type: ";
    if (event_type == STIMULATION_PULSE_EVENT) {
      std::cout << "Stimulation pulse event" << std::endl;
      std::cout << "Channel: " << +stimulation_pulse_event.channel << std::endl;
      std::cout << "Event info: " << std::endl;
      std::cout << "  Event id: " << stimulation_pulse_event.event_info.event_id << std::endl;
      std::cout << "  Execution condition: " << +stimulation_pulse_event.event_info.execution_condition << std::endl;
      std::cout << "  Time us: " << stimulation_pulse_event.event_info.time_us << std::endl;
      std::cout << "Pieces: " << std::endl;
      for (auto i = 0; i < 3; i++) {
        std::cout << "  mode: " << +stimulation_pulse_event.pieces[i].mode << ", duration in ticks: "
                  << stimulation_pulse_event.pieces[i].duration_in_ticks
                  << std::endl;
      }
    } else if (event_type == CHARGE_EVENT) {
      std::cout << "Charge event" << std::endl;
      std::cout << "Channel: " << +charge_event.channel << std::endl;
      std::cout << "Target voltage: " << charge_event.target_voltage << std::endl;
      std::cout << "Event info: " << std::endl;
      std::cout << "  Event id: " << charge_event.event_info.event_id << std::endl;
      std::cout << "  Execution condition: " << +charge_event.event_info.execution_condition << std::endl;
      std::cout << "  Time us: " << charge_event.event_info.time_us << std::endl;
    } else if (event_type == DISCHARGE_EVENT) {
      std::cout << "Discharge event" << std::endl;

      std::cout << "Channel: " << +discharge_event.channel << std::endl;
      std::cout << "Target voltage: " << discharge_event.target_voltage << std::endl;
      std::cout << "Event info: " << std::endl;
      std::cout << "  Event id: " << discharge_event.event_info.event_id << std::endl;
      std::cout << "  Execution condition: " << +discharge_event.event_info.execution_condition << std::endl;
      std::cout << "  Time us: " << discharge_event.event_info.time_us << std::endl;
    } else {
      std::cout << "Uknown event type " << +event_type << std::endl;
    }
  }

};

#endif //EEG_PROCESSOR_FPGA_EVENT_H
