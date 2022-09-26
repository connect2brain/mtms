//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// Thresholding.h
//
// Code generation for function 'Thresholding'
//

#ifndef THRESHOLDING_H
#define THRESHOLDING_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
class Thresholding {
public:
  Thresholding *init(const coder::array<double, 2U> &b_array,
                     unsigned short b_lag);
  double thresholding_algo();
  void enqueue_y();
  void enqueue_signals();
  void enqueue_filtered_y();
  void enqueue_avg_filter();
  void enqueue_std_filter();
  coder::array<double, 1U> signals;
  coder::array<double, 1U> avgFilter;
  coder::array<double, 1U> stdFilter;
  coder::array<double, 2U> filteredY;
  double length;
  coder::array<double, 2U> y;
  unsigned short lag;
  double threshold;
  double influence;
};

#endif
// End of code generation (Thresholding.h)
