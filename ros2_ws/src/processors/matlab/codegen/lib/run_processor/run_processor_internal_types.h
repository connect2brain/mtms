//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// run_processor_internal_types.h
//
// Code generation for function 'run_processor'
//

#ifndef RUN_PROCESSOR_INTERNAL_TYPES_H
#define RUN_PROCESSOR_INTERNAL_TYPES_H

// Include files
#include "rtwtypes.h"
#include "run_processor_types.h"

// Type Definitions
struct event_info {
  unsigned short event_id;
  unsigned char execution_condition;
  unsigned long time_us;
};

struct stimulation_pulse_piece {
  unsigned char mode;
  unsigned short duration_in_ticks;
};

struct matlab_fpga_event {
  unsigned char channel;
  event_info b_event_info;
  stimulation_pulse_piece pieces[3];
  unsigned char event_type;
  unsigned short target_voltage;
};

#endif
// End of code generation (run_processor_internal_types.h)
