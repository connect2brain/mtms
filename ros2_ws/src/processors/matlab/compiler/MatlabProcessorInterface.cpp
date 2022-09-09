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
#include "run_processor_types.h"
#include "coder_array.h"

// Variable Definitions
static const stimulation_pulse_event r{
    5U, // channel
    {
        0U,    // event_id
        false, // wait_for_trigger
        0UL    // time_us
    },         // b_event_info
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
};

// Function Definitions
void MatlabProcessorInterface::data_received(
    const double channel_data[62],
    coder::array<stimulation_pulse_event, 1U> &ret)
{
  double y;
  int i;
  int number_of_pulses;
  unsigned int q0;
  unsigned int qY;
  q0 = window_size;
  qY = q0 - 1U;
  if (q0 - 1U > q0) {
    qY = 0U;
  }
  i = static_cast<int>(qY);
  for (number_of_pulses = 0; number_of_pulses < i; number_of_pulses++) {
    data[number_of_pulses] = data[number_of_pulses + 1];
  }
  i = data.size(1) - 1;
  number_of_pulses = data.size(0);
  for (int i1{0}; i1 < number_of_pulses; i1++) {
    data[i1 + data.size(0) * i] = channel_data[i1];
  }
  //  a trick to allocate an array of structs
  //  Create pulse command
  y = channel_data[0];
  for (number_of_pulses = 0; number_of_pulses < 61; number_of_pulses++) {
    y += channel_data[number_of_pulses + 1];
  }
  if (y > 1.0) {
    number_of_pulses = 5;
  } else {
    number_of_pulses = 0;
  }
  pulses.set_size(number_of_pulses);
  for (i = 0; i < number_of_pulses; i++) {
    pulses[i] = r;
  }
  ret.set_size(pulses.size(0));
  number_of_pulses = pulses.size(0);
  for (i = 0; i < number_of_pulses; i++) {
    ret[i] = pulses[i];
  }
}

MatlabProcessorInterface *
MatlabProcessorInterface::init(unsigned int b_window_size,
                               unsigned short channel_count)
{
  MatlabProcessorInterface *obj;
  int number_of_pulses;
  obj = this;
  obj->data.set_size(static_cast<int>(channel_count),
                     static_cast<int>(b_window_size));
  number_of_pulses = channel_count * static_cast<int>(b_window_size);
  for (int i{0}; i < number_of_pulses; i++) {
    obj->data[i] = 0.0;
  }
  //  obj.data = zeros(1, window_size);
  obj->window_size = b_window_size;
  //  Create pulse command
  // display(pulse);
  number_of_pulses = 5;
  if (b_window_size < 5U) {
    number_of_pulses = 20;
  }
  obj->pulses.set_size(number_of_pulses);
  for (int i{0}; i < number_of_pulses; i++) {
    obj->pulses[i] = r;
  }
  return obj;
}

// End of code generation (MatlabProcessorInterface.cpp)
