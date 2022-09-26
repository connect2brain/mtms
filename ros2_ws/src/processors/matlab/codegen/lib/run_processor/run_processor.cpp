//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// run_processor.cpp
//
// Code generation for function 'run_processor'
//

// Include files
#include "run_processor.h"
#include "MatlabProcessor.h"
#include "rt_nonfinite.h"
#include "run_processor_internal_types.h"
#include "coder_array.h"
#include <cstring>

// Function Definitions
void run_processor(unsigned int window_size, unsigned short channel_count,
                   const double[62], unsigned long time_us,
                   boolean_T first_sample_of_experiment,
                   coder::array<double, 2U> &out)
{
  MatlabProcessor obj;
  coder::array<matlab_fpga_event, 1U> b_obj;
  coder::array<matlab_fpga_event, 1U> c_obj;
  coder::array<matlab_fpga_event, 1U> d_obj;
  coder::array<double, 2U> b_varargin_1;
  coder::array<double, 2U> varargin_1;
  double tmp_data[100];
  int actual_window_size;
  if (window_size < 50U) {
    actual_window_size = static_cast<int>(window_size);
  } else if (window_size > 100U) {
    actual_window_size = 40;
  } else {
    actual_window_size = static_cast<int>(window_size);
  }
  obj.init();
  obj.set_window_size(static_cast<unsigned int>(actual_window_size));
  obj.set_channel_count(channel_count);
  // obj.enqueue(data_sample);
  obj.init_experiment(b_obj);
  if (actual_window_size - 1 >= 0) {
    std::memset(&tmp_data[0], 0, actual_window_size * sizeof(double));
  }
  obj.data_received(tmp_data, actual_window_size, time_us,
                    first_sample_of_experiment, c_obj);
  //  display(data);
  obj.end_experiment(d_obj);
  //  out = data(2);
  if (window_size < 10U) {
    obj.AbstractMatlabProcessor_setData();
  } else {
    obj.setData();
  }
  actual_window_size = static_cast<int>(window_size);
  for (int i{0}; i < actual_window_size; i++) {
    int loop_ub;
    obj.getData(varargin_1);
    if ((varargin_1.size(0) != 0) && (varargin_1.size(1) != 0)) {
      loop_ub = varargin_1.size(1);
    } else {
      loop_ub = 0;
    }
    b_varargin_1.set_size(1, loop_ub + 1);
    for (int b_i{0}; b_i < loop_ub; b_i++) {
      b_varargin_1[b_i] = varargin_1[b_i];
    }
    b_varargin_1[loop_ub] = static_cast<double>(i) + 1.0;
    obj.setData(b_varargin_1);
  }
  obj.getData(out);
}

// End of code generation (run_processor.cpp)
