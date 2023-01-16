//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// run_processor_types.h
//
// Code generation for function 'run_processor'
//

#ifndef RUN_PROCESSOR_TYPES_H
#define RUN_PROCESSOR_TYPES_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <vector>

// Type Definitions
struct event_info {
  unsigned short id;
  unsigned char execution_condition;
  double execution_time;
};

struct waveform_piece {
  unsigned char waveform_phase;
  unsigned short duration_in_ticks;
};

/* Used by CompiledMatlabProcessor */
struct matlab_eeg_sample {
  coder::array<double, 2U> channel_data;
  double time;
  double first_sample_of_experiment;
};

/* Used by CppProcessor. Separate from matlab_eeg_sample so coder_array.h is not needed in cpp processors. */
struct eeg_sample {
  std::vector<double> channel_data;
  double time;
  double first_sample_of_experiment;
};

struct matlab_event {
  unsigned char channel;
  event_info b_event_info;
  waveform_piece waveform[3];
  unsigned char event_type;
  unsigned short target_voltage;
  unsigned int duration_us;
};

#endif
// End of code generation (run_processor_types.h)
