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
#include "Thresholding.h"
#include "blockedSummation.h"
#include "rt_nonfinite.h"
#include "run_processor_types.h"
#include "std.h"
#include "coder_array.h"
#include <cmath>

// Variable Definitions
static const stimulation_pulse_event r{
    5U, // channel
    {
        0U, // event_id
        0U, // execution_condition
        0UL // time_us
    },      // b_event_info
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
  coder::array<double, 1U> obj;
  double b_signal;
  int b_i;
  int i;
  int i1;
  int number_of_pulses;
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
  number_of_pulses = data.size(0);
  for (i1 = 0; i1 < number_of_pulses; i1++) {
    data[i1 + data.size(0) * i] = channel_data[i1];
  }
  // METHOD1 Summary of this method goes here
  //    Detailed explanation goes here
  peak_detection.y[peak_detection.y.size(0)] = channel_data[4];
  b_i = peak_detection.y.size(0);
  peak_detection.length = peak_detection.y.size(0);
  if (static_cast<unsigned int>(b_i) < peak_detection.lag) {
    b_signal = 0.0;
  } else if (static_cast<unsigned int>(b_i) == peak_detection.lag) {
    unsigned int b_qY;
    number_of_pulses = static_cast<int>(peak_detection.length);
    peak_detection.signals.set_size(number_of_pulses);
    for (i = 0; i < number_of_pulses; i++) {
      peak_detection.signals[i] = 0.0;
    }
    peak_detection.filteredY.set_size(peak_detection.y.size(0));
    number_of_pulses = peak_detection.y.size(0);
    for (i = 0; i < number_of_pulses; i++) {
      peak_detection.filteredY[i] = peak_detection.y[i];
    }
    q0 = peak_detection.lag;
    qY = q0 + 1U;
    if (q0 + 1U < q0) {
      qY = MAX_uint32_T;
    }
    number_of_pulses = static_cast<int>(qY);
    obj.set_size(static_cast<int>(qY));
    for (i = 0; i < number_of_pulses; i++) {
      obj[i] = peak_detection.y[i];
    }
    q0 = peak_detection.lag;
    b_qY = q0 + 1U;
    if (q0 + 1U < q0) {
      b_qY = MAX_uint32_T;
    }
    peak_detection.avgFilter[static_cast<int>(b_qY) - 1] =
        coder::blockedSummation(obj, static_cast<int>(qY)) /
        static_cast<double>(static_cast<int>(qY));
    q0 = peak_detection.lag;
    qY = q0 + 1U;
    if (q0 + 1U < q0) {
      qY = MAX_uint32_T;
    }
    number_of_pulses = static_cast<int>(qY);
    obj.set_size(static_cast<int>(qY));
    for (i = 0; i < number_of_pulses; i++) {
      obj[i] = peak_detection.y[i];
    }
    q0 = peak_detection.lag;
    qY = q0 + 1U;
    if (q0 + 1U < q0) {
      qY = MAX_uint32_T;
    }
    peak_detection.stdFilter[static_cast<int>(qY) - 1] = coder::b_std(obj);
    b_signal = 0.0;
  } else {
    peak_detection.signals[peak_detection.signals.size(0)] = 0.0;
    peak_detection.filteredY[peak_detection.filteredY.size(0)] = 0.0;
    peak_detection.avgFilter[peak_detection.avgFilter.size(0)] = 0.0;
    peak_detection.stdFilter[peak_detection.stdFilter.size(0)] = 0.0;
    if (std::abs(peak_detection.y[b_i - 1] -
                 peak_detection.avgFilter[b_i - 2]) >
        peak_detection.threshold * peak_detection.stdFilter[b_i - 2]) {
      if (peak_detection.y[b_i - 2] > peak_detection.avgFilter[b_i - 3]) {
        peak_detection.signals[b_i - 1] = 1.0;
      } else {
        peak_detection.signals[b_i - 1] = -1.0;
      }
      peak_detection.filteredY[b_i - 1] =
          peak_detection.influence * peak_detection.y[b_i - 1] +
          (1.0 - peak_detection.influence) * peak_detection.filteredY[b_i - 2];
      b_signal =
          static_cast<double>(b_i) - static_cast<double>(peak_detection.lag);
      if (b_signal >= 0.0) {
        q0 = static_cast<unsigned int>(b_signal);
      } else {
        q0 = 0U;
      }
      if (q0 > static_cast<unsigned int>(b_i)) {
        i = -1;
        i1 = -1;
      } else {
        i = static_cast<int>(q0) - 2;
        i1 = b_i - 1;
      }
      number_of_pulses = i1 - i;
      obj.set_size(number_of_pulses);
      for (i1 = 0; i1 < number_of_pulses; i1++) {
        obj[i1] = peak_detection.filteredY[(i + i1) + 1];
      }
      peak_detection.avgFilter[b_i - 1] =
          coder::blockedSummation(obj, number_of_pulses) /
          static_cast<double>(number_of_pulses);
      b_signal =
          static_cast<double>(b_i) - static_cast<double>(peak_detection.lag);
      if (b_signal >= 0.0) {
        q0 = static_cast<unsigned int>(b_signal);
      } else {
        q0 = 0U;
      }
      if (q0 > static_cast<unsigned int>(b_i)) {
        i = 0;
        i1 = 0;
      } else {
        i = static_cast<int>(q0) - 1;
        i1 = b_i;
      }
      number_of_pulses = i1 - i;
      obj.set_size(number_of_pulses);
      for (i1 = 0; i1 < number_of_pulses; i1++) {
        obj[i1] = peak_detection.filteredY[i + i1];
      }
      peak_detection.stdFilter[b_i - 1] = coder::b_std(obj);
    } else {
      peak_detection.signals[b_i - 1] = 0.0;
      b_signal =
          static_cast<double>(b_i) - static_cast<double>(peak_detection.lag);
      if (b_signal >= 0.0) {
        q0 = static_cast<unsigned int>(b_signal);
      } else {
        q0 = 0U;
      }
      if (q0 > static_cast<unsigned int>(b_i)) {
        i = -1;
        i1 = -1;
      } else {
        i = static_cast<int>(q0) - 2;
        i1 = b_i - 1;
      }
      number_of_pulses = i1 - i;
      obj.set_size(number_of_pulses);
      for (i1 = 0; i1 < number_of_pulses; i1++) {
        obj[i1] = peak_detection.filteredY[(i + i1) + 1];
      }
      peak_detection.avgFilter[b_i - 1] =
          coder::blockedSummation(obj, number_of_pulses) /
          static_cast<double>(number_of_pulses);
      if (b_signal >= 0.0) {
        q0 = static_cast<unsigned int>(b_signal);
      } else {
        q0 = 0U;
      }
      if (q0 > static_cast<unsigned int>(b_i)) {
        i = 0;
        i1 = 0;
      } else {
        i = static_cast<int>(q0) - 1;
        i1 = b_i;
      }
      number_of_pulses = i1 - i;
      obj.set_size(number_of_pulses);
      for (i1 = 0; i1 < number_of_pulses; i1++) {
        obj[i1] = peak_detection.filteredY[i + i1];
      }
      peak_detection.stdFilter[b_i - 1] = coder::b_std(obj);
    }
    b_signal = peak_detection.signals[b_i - 1];
  }
  //  Create pulse command
  number_of_pulses = (b_signal == 1.0);
  pulses.set_size(number_of_pulses);
  for (i = 0; i < number_of_pulses; i++) {
    pulses[0] = r;
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
  coder::array<double, 1U> x;
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
  obj->peak_detection.y.set_size(static_cast<int>(b_window_size));
  number_of_pulses = static_cast<int>(b_window_size);
  for (int i{0}; i < number_of_pulses; i++) {
    obj->peak_detection.y[i] = 0.0;
  }
  obj->peak_detection.lag = b_window_size;
  obj->peak_detection.threshold = 5.5;
  obj->peak_detection.influence = 0.5;
  number_of_pulses = obj->peak_detection.y.size(0);
  obj->peak_detection.signals.set_size(number_of_pulses);
  for (int i{0}; i < number_of_pulses; i++) {
    obj->peak_detection.signals[i] = 0.0;
  }
  //  Initialise filtered series
  if (b_window_size < 1U) {
    number_of_pulses = 0;
  } else {
    number_of_pulses = static_cast<int>(b_window_size);
  }
  obj->peak_detection.filteredY.set_size(number_of_pulses);
  for (int i{0}; i < number_of_pulses; i++) {
    obj->peak_detection.filteredY[i] = obj->peak_detection.y[i];
  }
  //  Initialise filters
  number_of_pulses = obj->peak_detection.y.size(0);
  obj->peak_detection.avgFilter.set_size(number_of_pulses);
  for (int i{0}; i < number_of_pulses; i++) {
    obj->peak_detection.avgFilter[i] = 0.0;
  }
  number_of_pulses = obj->peak_detection.y.size(0);
  obj->peak_detection.stdFilter.set_size(number_of_pulses);
  for (int i{0}; i < number_of_pulses; i++) {
    obj->peak_detection.stdFilter[i] = 0.0;
  }
  if (b_window_size < 1U) {
    number_of_pulses = 0;
  } else {
    number_of_pulses = static_cast<int>(b_window_size);
  }
  x.set_size(number_of_pulses);
  for (int i{0}; i < number_of_pulses; i++) {
    x[i] = obj->peak_detection.y[i];
  }
  obj->peak_detection.avgFilter[static_cast<int>(b_window_size) - 1] =
      coder::blockedSummation(x, x.size(0)) / static_cast<double>(x.size(0));
  number_of_pulses = static_cast<int>(b_window_size);
  x.set_size(static_cast<int>(b_window_size));
  for (int i{0}; i < number_of_pulses; i++) {
    x[i] = obj->peak_detection.y[i];
  }
  obj->peak_detection.stdFilter[static_cast<int>(b_window_size) - 1] =
      coder::b_std(x);
  return obj;
}

// End of code generation (MatlabProcessorInterface.cpp)
