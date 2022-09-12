//
// Created by alqio on 8.9.2022.
//

#ifndef DATA_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H
#define DATA_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H

#include "coder_array.h"
#include "run_processor_types.h"
#include <iostream>

class MatlabProcessorInterface {
public:
  virtual MatlabProcessorInterface *init(unsigned int b_window_size,
                                         unsigned short channel_count);

  virtual ~MatlabProcessorInterface() = default;

  virtual void data_received(const double channel_data[62],
                             coder::array<stimulation_pulse_event, 1U> &ret);
};

#endif //DATA_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H
