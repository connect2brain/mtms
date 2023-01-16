//
// Created by alqio on 1.9.2022.
//

#ifndef EEG_PROCESSOR_PROCESSOR_H
#define EEG_PROCESSOR_PROCESSOR_H

#include "string"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"

#include "fpga_event.h"


class ProcessorWrapper {
public:
  virtual std::vector<Event> init() = 0;

  virtual std::vector<Event> data_received(mtms_interfaces::msg::EegDatapoint data) = 0;

  virtual std::vector<Event> close() = 0;

};

#endif //EEG_PROCESSOR_PROCESSOR_H
