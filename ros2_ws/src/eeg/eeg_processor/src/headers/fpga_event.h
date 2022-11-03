//
// Created by alqio on 9/12/22.
//

#ifndef EEG_PROCESSOR_FPGA_EVENT_H
#define EEG_PROCESSOR_FPGA_EVENT_H

#include "fpga_interfaces/msg/pulse.hpp"
#include "fpga_interfaces/msg/charge.hpp"
#include "fpga_interfaces/msg/discharge.hpp"
#include "fpga_interfaces/msg/waveform_piece.hpp"
#include "fpga_interfaces/msg/waveform_phase.hpp"
#include <iostream>

enum FpgaEventType {
  PULSE = 0,
  CHARGE = 1,
  DISCHARGE = 2
};

struct FpgaEvent {
  fpga_interfaces::msg::Pulse pulse;
  fpga_interfaces::msg::Charge charge;
  fpga_interfaces::msg::Discharge discharge;
  FpgaEventType event_type = PULSE;

  [[nodiscard]] std::string to_string() const {
    if (event_type == PULSE) {
      return "Pulse";
    } else if (event_type == CHARGE) {
      return "Charge";
    } else {
      return "Discharge";
    }
  }

  void print() {
    std::cout << "Event type: ";
    if (event_type == PULSE) {
      std::cout << "Pulse" << std::endl;
      std::cout << "Channel: " << +pulse.channel << std::endl;
      std::cout << "Event: " << std::endl;
      std::cout << "  Id: " << pulse.event.id << std::endl;
      std::cout << "  Execution condition: " << +pulse.event.execution_condition.value << std::endl;
      std::cout << "  Time us: " << pulse.event.time_us << std::endl;
      std::cout << "Waveform: " << std::endl;
      for (auto i = 0; i < 3; i++) {
        std::cout << "  Phase: " << +pulse.waveform[i].waveform_phase.value << ", duration in ticks: "
                  << pulse.waveform[i].duration_in_ticks
                  << std::endl;
      }
    } else if (event_type == CHARGE) {
      std::cout << "Charge" << std::endl;
      std::cout << "Channel: " << +charge.channel << std::endl;
      std::cout << "Target voltage: " << charge.target_voltage << std::endl;
      std::cout << "Event: " << std::endl;
      std::cout << "  Id: " << charge.event.id << std::endl;
      std::cout << "  Execution condition: " << +charge.event.execution_condition.value << std::endl;
      std::cout << "  Time us: " << charge.event.time_us << std::endl;
    } else if (event_type == DISCHARGE) {
      std::cout << "Discharge" << std::endl;

      std::cout << "Channel: " << +discharge.channel << std::endl;
      std::cout << "Target voltage: " << discharge.target_voltage << std::endl;
      std::cout << "Event: " << std::endl;
      std::cout << "  Id: " << discharge.event.id << std::endl;
      std::cout << "  Execution condition: " << +discharge.event.execution_condition.value << std::endl;
      std::cout << "  Time us: " << discharge.event.time_us << std::endl;
    } else {
      std::cout << "Uknown event type " << +event_type << std::endl;
    }
  }

};

#endif //EEG_PROCESSOR_FPGA_EVENT_H
