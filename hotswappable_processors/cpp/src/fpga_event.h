//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_FACTORY_FPGA_EVENT_H
#define PROCESSOR_FACTORY_FPGA_EVENT_H

struct event_info {
  unsigned short id;
  unsigned char execution_condition;
  double execution_time;
};

struct waveform_piece {
  unsigned char waveform_phase;
  unsigned short duration_in_ticks;
};

struct fpga_event {
  unsigned char channel;
  event_info b_event_info;
  waveform_piece waveform[3];
  unsigned char event_type;
  unsigned short target_voltage;
  unsigned int duration_us;
};


#endif //PROCESSOR_FACTORY_FPGA_EVENT_H
