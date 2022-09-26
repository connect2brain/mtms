//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// std.cpp
//
// Code generation for function 'std'
//

// Include files
#include "std.h"
#include "rt_nonfinite.h"
#include "coder_array.h"
#include <cmath>

// Function Definitions
namespace coder {
double b_std(const ::coder::array<double, 2U> &x)
{
  array<double, 1U> absdiff;
  double y;
  int n;
  n = x.size(1);
  if (x.size(1) == 0) {
    y = rtNaN;
  } else if (x.size(1) == 1) {
    if ((!std::isinf(x[0])) && (!std::isnan(x[0]))) {
      y = 0.0;
    } else {
      y = rtNaN;
    }
  } else {
    double bsum;
    double xbar;
    int firstBlockLength;
    int lastBlockLength;
    int nblocks;
    if (x.size(1) <= 1024) {
      firstBlockLength = x.size(1);
      lastBlockLength = 0;
      nblocks = 1;
    } else {
      firstBlockLength = 1024;
      nblocks = x.size(1) / 1024;
      lastBlockLength = x.size(1) - (nblocks << 10);
      if (lastBlockLength > 0) {
        nblocks++;
      } else {
        lastBlockLength = 1024;
      }
    }
    xbar = x[0];
    for (int k{2}; k <= firstBlockLength; k++) {
      xbar += x[k - 1];
    }
    for (int ib{2}; ib <= nblocks; ib++) {
      int hi;
      firstBlockLength = (ib - 1) << 10;
      bsum = x[firstBlockLength];
      if (ib == nblocks) {
        hi = lastBlockLength;
      } else {
        hi = 1024;
      }
      for (int k{2}; k <= hi; k++) {
        bsum += x[(firstBlockLength + k) - 1];
      }
      xbar += bsum;
    }
    xbar /= static_cast<double>(x.size(1));
    absdiff.set_size(x.size(1));
    for (int k{0}; k < n; k++) {
      absdiff[k] = std::abs(x[k] - xbar);
    }
    y = 0.0;
    xbar = 3.3121686421112381E-170;
    firstBlockLength = x.size(1);
    for (int k{0}; k < firstBlockLength; k++) {
      if (absdiff[k] > xbar) {
        bsum = xbar / absdiff[k];
        y = y * bsum * bsum + 1.0;
        xbar = absdiff[k];
      } else {
        bsum = absdiff[k] / xbar;
        y += bsum * bsum;
      }
    }
    y = xbar * std::sqrt(y);
    y /= std::sqrt(static_cast<double>(x.size(1)) - 1.0);
  }
  return y;
}

} // namespace coder

// End of code generation (std.cpp)
