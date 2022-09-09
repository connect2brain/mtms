//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// MatlabProcessorInterface.h
//
// Code generation for function 'MatlabProcessorInterface'
//

#ifndef MATLABPROCESSORINTERFACE_H
#define MATLABPROCESSORINTERFACE_H

// Include files
#include "rtwtypes.h"
#include "run_processor_types.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
class MatlabProcessorInterface {
public:
  virtual MatlabProcessorInterface *init(unsigned int b_window_size,
                                 unsigned short channel_count);
  virtual void data_received(const double channel_data[62],
                     coder::array<struct0_T, 1U> &ret);

private:
  virtual void enqueue(const double element[62]);

public:
  coder::array<double, 2U> data;
  unsigned int window_size;
  coder::array<struct0_T, 1U> pulses;
};

#endif
// End of code generation (MatlabProcessorInterface.h)
