//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// create_command.cpp
//
// Code generation for function 'create_command'
//

// Include files
#include "create_command.h"
#include "rt_nonfinite.h"
#include "run_processor_internal_types.h"

// Function Definitions
void b_create_command(unsigned int event_id, unsigned char *command_channel,
                      unsigned short *command_event_info_event_id,
                      unsigned char *command_event_info_execution_condition,
                      unsigned long *command_event_info_time_us,
                      stimulation_pulse_piece command_pieces[3],
                      unsigned char *command_event_type,
                      unsigned short *command_target_voltage)
{
  unsigned int u;
  u = event_id;
  if (event_id > 65535U) {
    u = 65535U;
  }
  *command_event_info_event_id = static_cast<unsigned short>(u);
  *command_event_info_execution_condition = 2U;
  *command_event_info_time_us = 69999UL;
  command_pieces[0].mode = 0U;
  command_pieces[0].duration_in_ticks = 200U;
  command_pieces[1].mode = 2U;
  command_pieces[1].duration_in_ticks = 269U;
  command_pieces[2].mode = 1U;
  command_pieces[2].duration_in_ticks = 1U;
  *command_channel = 5U;
  *command_event_type = 1U;
  *command_target_voltage = 1200U;
}

void create_command(unsigned int event_id, unsigned char *command_channel,
                    unsigned short *command_event_info_event_id,
                    unsigned char *command_event_info_execution_condition,
                    unsigned long *command_event_info_time_us,
                    stimulation_pulse_piece command_pieces[3],
                    unsigned char *command_event_type,
                    unsigned short *command_target_voltage)
{
  unsigned int u;
  u = event_id;
  if (event_id > 65535U) {
    u = 65535U;
  }
  *command_event_info_event_id = static_cast<unsigned short>(u);
  *command_event_info_execution_condition = 2U;
  *command_event_info_time_us = 69999UL;
  command_pieces[0].mode = 0U;
  command_pieces[0].duration_in_ticks = 200U;
  command_pieces[1].mode = 2U;
  command_pieces[1].duration_in_ticks = 269U;
  command_pieces[2].mode = 1U;
  command_pieces[2].duration_in_ticks = 1U;
  *command_channel = 5U;
  *command_event_type = 0U;
  *command_target_voltage = 0U;
}

// End of code generation (create_command.cpp)
