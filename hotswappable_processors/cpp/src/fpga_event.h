//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_FACTORY_FPGA_EVENT_H
#define PROCESSOR_FACTORY_FPGA_EVENT_H

#include <vector>

struct event {
  unsigned short id;
  unsigned char execution_condition;
  double time;
};

struct waveform_piece {
  unsigned char waveform_phase;
  unsigned short duration_in_ticks;
};

struct eeg_sample {
  std::vector<double> channel_data;
  double time;
  double first_sample_of_experiment;
};

struct fpga_event {
  unsigned char channel;
  event b_event;
  waveform_piece waveform[3];
  unsigned char event_type;
  unsigned short target_voltage;
  unsigned int duration_us;
};


#endif //PROCESSOR_FACTORY_FPGA_EVENT_H
