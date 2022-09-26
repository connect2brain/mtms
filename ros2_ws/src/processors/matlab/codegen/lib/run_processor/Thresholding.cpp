//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// Thresholding.cpp
//
// Code generation for function 'Thresholding'
//

// Include files
#include "Thresholding.h"
#include "mean.h"
#include "rt_nonfinite.h"
#include "std.h"
#include "coder_array.h"
#include <cmath>

// Function Definitions
void Thresholding::enqueue_avg_filter()
{
  int i;
  i = static_cast<int>(length - 1.0);
  for (int b_i{0}; b_i < i; b_i++) {
    avgFilter[b_i] = avgFilter[b_i + 1];
  }
  avgFilter[avgFilter.size(0) - 1] = 0.0;
}

void Thresholding::enqueue_filtered_y()
{
  int i;
  i = static_cast<int>(length - 1.0);
  for (int b_i{0}; b_i < i; b_i++) {
    filteredY[b_i] = filteredY[b_i + 1];
  }
  filteredY[filteredY.size(0) * filteredY.size(1) - 1] = 0.0;
}

void Thresholding::enqueue_signals()
{
  int i;
  i = static_cast<int>(length - 1.0);
  for (int b_i{0}; b_i < i; b_i++) {
    signals[b_i] = signals[b_i + 1];
  }
  signals[signals.size(0) - 1] = 0.0;
}

void Thresholding::enqueue_std_filter()
{
  int i;
  i = static_cast<int>(length - 1.0);
  for (int b_i{0}; b_i < i; b_i++) {
    stdFilter[b_i] = stdFilter[b_i + 1];
  }
  stdFilter[stdFilter.size(0) - 1] = 0.0;
}

void Thresholding::enqueue_y()
{
  int i;
  i = static_cast<int>(length - 1.0);
  for (int b_i{0}; b_i < i; b_i++) {
    y[b_i] = y[b_i + 1];
  }
  y[y.size(0) * y.size(1) - 1] = 0.0;
}

Thresholding *Thresholding::init(const coder::array<double, 2U> &b_array,
                                 unsigned short b_lag)
{
  Thresholding *obj;
  coder::array<double, 2U> b_obj;
  int loop_ub;
  int varargin_2;
  obj = this;
  //  coder.inline("always");
  obj->y.set_size(b_array.size(0), b_array.size(1));
  loop_ub = b_array.size(0) * b_array.size(1);
  for (varargin_2 = 0; varargin_2 < loop_ub; varargin_2++) {
    obj->y[varargin_2] = b_array[varargin_2];
  }
  loop_ub = obj->y.size(0);
  varargin_2 = obj->y.size(1);
  if ((loop_ub == 0) || (varargin_2 == 0)) {
    varargin_2 = 0;
  } else if (loop_ub >= varargin_2) {
    varargin_2 = loop_ub;
  }
  obj->length = varargin_2;
  obj->lag = b_lag;
  obj->threshold = 4.5;
  obj->influence = 0.5;
  obj->filteredY.set_size(b_array.size(0), b_array.size(1));
  loop_ub = b_array.size(0) * b_array.size(1);
  for (varargin_2 = 0; varargin_2 < loop_ub; varargin_2++) {
    obj->filteredY[varargin_2] = b_array[varargin_2];
  }
  loop_ub = static_cast<int>(obj->length);
  obj->signals.set_size(loop_ub);
  for (varargin_2 = 0; varargin_2 < loop_ub; varargin_2++) {
    obj->signals[varargin_2] = 0.0;
  }
  loop_ub = static_cast<int>(obj->length);
  obj->avgFilter.set_size(loop_ub);
  for (varargin_2 = 0; varargin_2 < loop_ub; varargin_2++) {
    obj->avgFilter[varargin_2] = 0.0;
  }
  loop_ub = static_cast<int>(obj->length);
  obj->stdFilter.set_size(loop_ub);
  for (varargin_2 = 0; varargin_2 < loop_ub; varargin_2++) {
    obj->stdFilter[varargin_2] = 0.0;
  }
  if (b_lag < 1) {
    loop_ub = 0;
  } else {
    loop_ub = b_lag;
  }
  b_obj.set_size(1, loop_ub);
  for (varargin_2 = 0; varargin_2 < loop_ub; varargin_2++) {
    b_obj[varargin_2] = obj->y[varargin_2];
  }
  obj->avgFilter[b_lag - 1] = coder::mean(b_obj);
  loop_ub = b_lag;
  b_obj.set_size(1, static_cast<int>(b_lag));
  for (varargin_2 = 0; varargin_2 < loop_ub; varargin_2++) {
    b_obj[varargin_2] = obj->y[varargin_2];
  }
  obj->stdFilter[b_lag - 1] = coder::b_std(b_obj);
  return obj;
}

double Thresholding::thresholding_algo()
{
  coder::array<double, 2U> obj;
  double i;
  //  coder.inline("always");
  //  obj.y(end + 1) = new_value;
  enqueue_y();
  enqueue_signals();
  enqueue_filtered_y();
  enqueue_avg_filter();
  enqueue_std_filter();
  i = length;
  // fprintf("%f\n", new_value);
  if (std::abs(y[static_cast<int>(i) - 1] -
               avgFilter[static_cast<int>(i) - 2]) >
      threshold * stdFilter[static_cast<int>(i) - 2]) {
    double d;
    int b_i;
    int i1;
    int loop_ub;
    unsigned short u;
    if (y[static_cast<int>(i) - 1] > avgFilter[static_cast<int>(i) - 2]) {
      signals[static_cast<int>(i) - 1] = 1.0;
    } else {
      signals[static_cast<int>(i) - 1] = -1.0;
    }
    filteredY[static_cast<int>(i) - 1] =
        influence * y[static_cast<int>(i) - 1] +
        (1.0 - influence) * filteredY[static_cast<int>(i) - 2];
    d = std::round(i - static_cast<double>(lag));
    if (d < 65536.0) {
      if (d >= 0.0) {
        u = static_cast<unsigned short>(d);
      } else {
        u = 0U;
      }
    } else if (d >= 65536.0) {
      u = MAX_uint16_T;
    } else {
      u = 0U;
    }
    if (u > static_cast<unsigned short>(static_cast<int>(i))) {
      b_i = 0;
      i1 = 0;
    } else {
      b_i = u - 1;
      i1 = static_cast<unsigned short>(static_cast<int>(i));
    }
    loop_ub = i1 - b_i;
    obj.set_size(1, loop_ub);
    for (i1 = 0; i1 < loop_ub; i1++) {
      obj[i1] = filteredY[b_i + i1];
    }
    avgFilter[static_cast<int>(i) - 1] = coder::mean(obj);
    if (d < 65536.0) {
      if (d >= 0.0) {
        u = static_cast<unsigned short>(d);
      } else {
        u = 0U;
      }
    } else if (d >= 65536.0) {
      u = MAX_uint16_T;
    } else {
      u = 0U;
    }
    if (u > static_cast<unsigned short>(i)) {
      b_i = 0;
      i1 = 0;
    } else {
      b_i = u - 1;
      i1 = static_cast<unsigned short>(i);
    }
    loop_ub = i1 - b_i;
    obj.set_size(1, loop_ub);
    for (i1 = 0; i1 < loop_ub; i1++) {
      obj[i1] = filteredY[b_i + i1];
    }
    stdFilter[static_cast<int>(i) - 1] = coder::b_std(obj);
  } else {
    double d;
    int b_i;
    int i1;
    int loop_ub;
    unsigned short u;
    signals[static_cast<int>(i) - 1] = 0.0;
    filteredY.set_size(1, 1);
    filteredY[0] = y[static_cast<int>(i) - 1];
    d = std::round(i - static_cast<double>(lag));
    if (d < 65536.0) {
      if (d >= 0.0) {
        u = static_cast<unsigned short>(d);
      } else {
        u = 0U;
      }
    } else if (d >= 65536.0) {
      u = MAX_uint16_T;
    } else {
      u = 0U;
    }
    if (u > static_cast<unsigned short>(static_cast<int>(i))) {
      b_i = 0;
      i1 = 0;
    } else {
      b_i = u - 1;
      i1 = static_cast<unsigned short>(static_cast<int>(i));
    }
    loop_ub = i1 - b_i;
    obj.set_size(1, loop_ub);
    for (i1 = 0; i1 < loop_ub; i1++) {
      obj[i1] = filteredY[b_i + i1];
    }
    avgFilter[static_cast<int>(i) - 1] = coder::mean(obj);
    if (d < 65536.0) {
      if (d >= 0.0) {
        u = static_cast<unsigned short>(d);
      } else {
        u = 0U;
      }
    } else if (d >= 65536.0) {
      u = MAX_uint16_T;
    } else {
      u = 0U;
    }
    if (u > static_cast<unsigned short>(i)) {
      b_i = 0;
      i1 = 0;
    } else {
      b_i = u - 1;
      i1 = static_cast<unsigned short>(i);
    }
    loop_ub = i1 - b_i;
    obj.set_size(1, loop_ub);
    for (i1 = 0; i1 < loop_ub; i1++) {
      obj[i1] = filteredY[b_i + i1];
    }
    stdFilter[static_cast<int>(i) - 1] = coder::b_std(obj);
  }
  //  fprintf("y %f, signals %f, filteredY %f, avgFilter %f, stdFilter %f\n",
  //  obj.y(end), obj.signals(end), obj.filteredY(end), obj.avgFilter(end),
  //  obj.stdFilter(end));
  return signals[static_cast<int>(i) - 1];
}

// End of code generation (Thresholding.cpp)
