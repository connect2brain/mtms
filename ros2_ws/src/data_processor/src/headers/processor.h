//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_PROCESSOR_H
#define DATA_PROCESSOR_PROCESSOR_H

#include "string"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"

class ProcessorWrapper {
public:
  virtual void init() = 0;

  virtual std::vector<fpga_interfaces::msg::StimulationPulseEvent>
  data_received(mtms_interfaces::msg::EegDatapoint data) = 0;

  virtual int close() = 0;

};


#endif //DATA_PROCESSOR_PROCESSOR_H
