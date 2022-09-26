//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// _coder_run_processor_api.h
//
// Code generation for function 'run_processor'
//

#ifndef _CODER_RUN_PROCESSOR_API_H
#define _CODER_RUN_PROCESSOR_API_H

// Include files
#include "coder_array_mex.h"
#include "emlrt.h"
#include "tmwtypes.h"
#include <algorithm>
#include <cstring>

// Variable Declarations
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

// Function Declarations
void run_processor(uint32_T window_size, uint16_T channel_count,
                   real_T data_sample[62], uint64_T time_us,
                   boolean_T first_sample_of_experiment,
                   coder::array<real_T, 2U> *out);

void run_processor_api(const mxArray *const prhs[5], const mxArray **plhs);

void run_processor_atexit();

void run_processor_initialize();

void run_processor_terminate();

void run_processor_xil_shutdown();

void run_processor_xil_terminate();

#endif
// End of code generation (_coder_run_processor_api.h)
