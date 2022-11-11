//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_FACTORY_FPGA_EVENT_H
#define PROCESSOR_FACTORY_FPGA_EVENT_H

struct waveform_piece {
  unsigned char waveform_phase;
  unsigned short duration_in_ticks;
};

struct event {
  unsigned short id;
  unsigned char execution_condition;
  double time;
};

struct struct_T {
  int xstart;
  int xend;
  int depth;
};

struct fpga_event {
  unsigned char channel;
  event event_info;
  waveform_piece waveform[3];
  unsigned char event_type;
  unsigned short target_voltage;
};


#endif //PROCESSOR_FACTORY_FPGA_EVENT_H
