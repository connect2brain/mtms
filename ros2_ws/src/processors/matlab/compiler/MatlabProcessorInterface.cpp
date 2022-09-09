//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// MatlabProcessorInterface.cpp
//
// Code generation for function 'MatlabProcessorInterface'
//

// Include files
#include "MatlabProcessorInterface.h"
#include "rand.h"
#include "run_processor_types.h"
#include "coder_array.h"
#include <cmath>

// Variable Definitions
static const struct0_T r{
    5.0, // channel
    {
        0U,    // event_id
        false, // wait_for_trigger
        0UL    // time_us
    },         // event_info
    {
        {{
             0U,  // mode
             200U // duration_in_ticks
         },
         {
             2U,  // mode
             160U // duration_in_ticks
         },
         {
             1U,  // mode
             160U // duration_in_ticks
         }}       // pieces
    }             // pulse_info
};

// Function Definitions
void MatlabProcessorInterface::enqueue(const double element[62])
{
  int b_i;
  int i;
  unsigned int q0;
  unsigned int qY;
  q0 = window_size;
  qY = q0 - 1U;
  if (q0 - 1U > q0) {
    qY = 0U;
  }
  i = static_cast<int>(qY);
  for (b_i = 0; b_i < i; b_i++) {
    data[b_i] = data[b_i + 1];
  }
  i = data.size(1) - 1;
  b_i = data.size(0);
  for (int i1{0}; i1 < b_i; i1++) {
    data[i1 + data.size(0) * i] = element[i1];
  }
}

void MatlabProcessorInterface::data_received(const double channel_data[62],
                                             coder::array<struct0_T, 1U> &ret)
{
  double d;
  int varargin_1;
  enqueue(channel_data);
  //  a trick to allocate an array of structs
  //  Create pulse command
  d = std::round(15.0 * coder::b_rand());
  if (d < 2.147483648E+9) {
    if (d >= -2.147483648E+9) {
      varargin_1 = static_cast<int>(d);
    } else {
      varargin_1 = MIN_int32_T;
    }
  } else if (d >= 2.147483648E+9) {
    varargin_1 = MAX_int32_T;
  } else {
    varargin_1 = 0;
  }
  pulses.set_size(static_cast<int>(static_cast<signed char>(varargin_1)));
  varargin_1 = static_cast<signed char>(varargin_1);
  for (int i{0}; i < varargin_1; i++) {
    pulses[i] = r;
  }
  ret.set_size(pulses.size(0));
  varargin_1 = pulses.size(0);
  for (int i{0}; i < varargin_1; i++) {
    ret[i] = pulses[i];
  }
}

MatlabProcessorInterface *
MatlabProcessorInterface::init(unsigned int b_window_size,
                               unsigned short channel_count)
{
  MatlabProcessorInterface *obj;
  double d;
  int loop_ub;
  obj = this;
  obj->data.set_size(static_cast<int>(channel_count),
                     static_cast<int>(b_window_size));
  loop_ub = channel_count * static_cast<int>(b_window_size);
  for (int i{0}; i < loop_ub; i++) {
    obj->data[i] = 0.0;
  }
  //  obj.data = zeros(1, window_size);
  obj->window_size = b_window_size;
  //  Create pulse command
  d = std::round(15.0 * coder::b_rand());
  if (d < 2.147483648E+9) {
    if (d >= -2.147483648E+9) {
      loop_ub = static_cast<int>(d);
    } else {
      loop_ub = MIN_int32_T;
    }
  } else if (d >= 2.147483648E+9) {
    loop_ub = MAX_int32_T;
  } else {
    loop_ub = 0;
  }
  obj->pulses.set_size(static_cast<int>(static_cast<signed char>(loop_ub)));
  loop_ub = static_cast<signed char>(loop_ub);
  for (int i{0}; i < loop_ub; i++) {
    obj->pulses[i] = r;
  }
  return obj;
}

// End of code generation (MatlabProcessorInterface.cpp)
