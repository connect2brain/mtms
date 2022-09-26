//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// MatlabProcessor.h
//
// Code generation for function 'MatlabProcessor'
//

#ifndef MATLABPROCESSOR_H
#define MATLABPROCESSOR_H

// Include files
#include "Thresholding.h"
#include "rtwtypes.h"
#include "run_processor_internal_types.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
class MatlabProcessor {
public:
  MatlabProcessor *init();
  void constructor(Thresholding *iobj_0);
  void set_window_size(unsigned int new_window_size);
  void set_channel_count(unsigned short new_channel_count);
  void init_experiment(coder::array<matlab_fpga_event, 1U> &ret);
  void on_init_experiment();
  void data_received(const double channel_data_data[], int channel_data_size,
                     unsigned long time_us,
                     boolean_T first_sample_of_experiment,
                     coder::array<matlab_fpga_event, 1U> &ret);
  void enqueue(const double element_data[], int element_size);
  void on_data_received();
  void set_commands(const matlab_fpga_event b_commands[2]);
  void end_experiment(coder::array<matlab_fpga_event, 1U> &ret);
  MatlabProcessor *setData();
  void getData(coder::array<double, 2U> &val) const;
  MatlabProcessor *setData(const coder::array<double, 2U> &val);
  MatlabProcessor *AbstractMatlabProcessor_setData();
  void set_channel_count();
  void set_window_size();
  void set_auto_enqueue();

private:
  MatlabProcessor *AbstractMatlabProcessor_init();

public:
  coder::array<double, 2U> data;
  unsigned short window_size;
  unsigned short channel_count;
  coder::array<matlab_fpga_event, 1U> commands;
  unsigned int events_sent;
  boolean_T experiment_in_progress;
  unsigned long last_sample_received_at_us;
  boolean_T auto_enqueue;
  Thresholding _pobj0;

private:
  Thresholding *peak_detection;
};

#endif
// End of code generation (MatlabProcessor.h)
