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
  virtual ~MatlabProcessorInterface() = default;

  virtual MatlabProcessorInterface *init();

  virtual void init_experiment(coder::array<matlab_fpga_event, 1U> &ret);

  virtual void end_experiment(coder::array<matlab_fpga_event, 1U> &ret);

  virtual void data_received(const double channel_data_data[], int channel_data_size,
                             unsigned long time_us,
                             boolean_T first_sample_of_experiment,
                             coder::array<matlab_fpga_event, 1U> &ret);

};

using create_processor = MatlabProcessorInterface *(*)();

#endif //DATA_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H
