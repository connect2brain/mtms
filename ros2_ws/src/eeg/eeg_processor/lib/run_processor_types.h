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

// Type Definitions
struct event {
  unsigned short id;
  unsigned char execution_condition;
  unsigned long time_us;
};

struct pulse_piece {
  unsigned char waveform_phase;
  unsigned short duration_in_ticks;
};

struct matlab_fpga_event {
  unsigned char channel;
  event b_event;
  pulse_piece pieces[3];
  unsigned char event_type;
  unsigned short target_voltage;
};

#endif
// End of code generation (run_processor_types.h)
