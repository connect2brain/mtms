//
// Created by alqio on 9/12/22.
//

#ifndef EEG_PROCESSOR_FPGA_EVENT_H
#define EEG_PROCESSOR_FPGA_EVENT_H

#include "event_interfaces/msg/pulse.hpp"
#include "event_interfaces/msg/charge.hpp"
#include "event_interfaces/msg/discharge.hpp"
#include "event_interfaces/msg/waveform_piece.hpp"
#include "event_interfaces/msg/waveform_phase.hpp"
#include "event_interfaces/msg/signal_out.hpp"
#include <iostream>

enum EventType {
  PULSE = 0,
  CHARGE = 1,
  DISCHARGE = 2,
  SIGNAL_OUT = 3
};

struct Event {
  event_interfaces::msg::Pulse pulse;
  event_interfaces::msg::Charge charge;
  event_interfaces::msg::Discharge discharge;
  event_interfaces::msg::SignalOut signal_out;
  EventType event_type = PULSE;

  [[nodiscard]] std::string to_string() const {
    if (event_type == PULSE) {
      return "Pulse";
    } else if (event_type == CHARGE) {
      return "Charge";
    } else if (event_type == SIGNAL_OUT) {
      return "Signal out";
    } else {
      return "Discharge";
    }
  }

  void print() {
    std::cout << "Event type: ";
    if (event_type == PULSE) {
      std::cout << "Pulse" << std::endl;
      std::cout << "Channel: " << +pulse.channel << std::endl;
      std::cout << "Event info: " << std::endl;
      std::cout << "  Id: " << pulse.event_info.id << std::endl;
      std::cout << "  Execution condition: " << +pulse.event_info.execution_condition.value << std::endl;
      std::cout << "  Execution time: " << pulse.event_info.execution_time << std::endl;
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
      std::cout << "Event info: " << std::endl;
      std::cout << "  Id: " << charge.event_info.id << std::endl;
      std::cout << "  Execution condition: " << +charge.event_info.execution_condition.value << std::endl;
      std::cout << "  Execution time: " << charge.event_info.execution_time << std::endl;
    } else if (event_type == DISCHARGE) {
      std::cout << "Discharge" << std::endl;

      std::cout << "Channel: " << +discharge.channel << std::endl;
      std::cout << "Target voltage: " << discharge.target_voltage << std::endl;
      std::cout << "Event info: " << std::endl;
      std::cout << "  Id: " << discharge.event_info.id << std::endl;
      std::cout << "  Execution condition: " << +discharge.event_info.execution_condition.value << std::endl;
      std::cout << "  Execution time: " << discharge.event_info.execution_time << std::endl;
    } else if (event_type == SIGNAL_OUT) {
      std::cout << "Signal out" << std::endl;

      std::cout << "Port: " << +signal_out.port << std::endl;
      std::cout << "Event info: " << std::endl;
      std::cout << "  Id: " << signal_out.event_info.id << std::endl;
      std::cout << "  Execution condition: " << +signal_out.event_info.execution_condition.value << std::endl;
      std::cout << "  Execution time: " << signal_out.event_info.execution_time << std::endl;
    } else {
      std::cout << "Unknown event type " << +event_type << std::endl;
    }
  }

};

#endif //EEG_PROCESSOR_FPGA_EVENT_H
