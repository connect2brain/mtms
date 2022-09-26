//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// run_processor.h
//
// Code generation for function 'run_processor'
//

#ifndef RUN_PROCESSOR_H
#define RUN_PROCESSOR_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
extern void run_processor(unsigned int window_size,
                          unsigned short channel_count,
                          const double data_sample[62], unsigned long time_us,
                          boolean_T first_sample_of_experiment,
                          coder::array<double, 2U> &out);

#endif
// End of code generation (run_processor.h)
