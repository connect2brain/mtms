//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_PROCESSOR_H
#define DATA_PROCESSOR_PROCESSOR_H

#include "string"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"

#include "fpga_event.h"


class ProcessorWrapper {
public:
  virtual std::vector<FpgaEvent> init() = 0;

  virtual std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data) = 0;

  virtual std::vector<FpgaEvent> close() = 0;

};

#endif //DATA_PROCESSOR_PROCESSOR_H
