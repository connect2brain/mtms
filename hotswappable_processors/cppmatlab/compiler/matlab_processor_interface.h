//
// Created by alqio on 8.9.2022.
//

#ifndef DATA_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H
#define DATA_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H

#include "coder_array.h"
#include "run_processor_internal_types.h"
#include <iostream>

//The same class can be found from ros2_ws/src/realtime/eeg_processor/src/headers/. The method order and signature must be
//exactly the same as here. If in wrong order, calling a method will call the wrong method


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
#endif //DATA_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H
