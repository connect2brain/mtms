//
// Created by alqio on 8.9.2022.
//

#ifndef EEG_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H
#define EEG_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H

#include "coder_array.h"
#include "run_processor_types.h"
#include <iostream>


//The same class can be found from pipeline/. The method order and signature must be
//exactly the same as here. If in wrong order, calling a method will call the wrong method

class MatlabProcessorInterface {
public:
  virtual ~MatlabProcessorInterface() = default;

  virtual MatlabProcessorInterface *init();

  virtual void init_experiment(coder::array<matlab_event, 1U> &ret);

  virtual void end_experiment(coder::array<matlab_event, 1U> &ret);

  virtual void data_received(const double channel_data_data[], int channel_data_size,
                             double time,
                             boolean_T first_sample_of_experiment,
                             coder::array<matlab_event, 1U> &ret,
                             coder::array<matlab_eeg_sample, 1U> &b_samples);

};

using create_processor = MatlabProcessorInterface *(*)();

#endif //EEG_PROCESSOR_MATLAB_PROCESSOR_INTERFACE_H
