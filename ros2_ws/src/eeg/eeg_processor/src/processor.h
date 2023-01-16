//
// Created by alqio on 1.9.2022.
//

#ifndef EEG_PROCESSOR_PROCESSOR_H
#define EEG_PROCESSOR_PROCESSOR_H

#include "string"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "mtms_interfaces/msg/event.hpp"

#include "fpga_event.h"

template <class InputType, class OutputType>
class ProcessorWrapper {
public:
  virtual std::vector<OutputType> init() = 0;

  virtual std::vector<OutputType> eeg_received(mtms_interfaces::msg::EegDatapoint sample) = 0;

  virtual std::vector<OutputType> event_received(mtms_interfaces::msg::Event event) = 0;

  virtual void close() = 0;

};

#endif //EEG_PROCESSOR_PROCESSOR_H
