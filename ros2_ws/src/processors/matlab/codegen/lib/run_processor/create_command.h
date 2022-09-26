//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// create_command.h
//
// Code generation for function 'create_command'
//

#ifndef CREATE_COMMAND_H
#define CREATE_COMMAND_H

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Type Declarations
struct stimulation_pulse_piece;

// Function Declarations
void b_create_command(unsigned int event_id, unsigned char *command_channel,
                      unsigned short *command_event_info_event_id,
                      unsigned char *command_event_info_execution_condition,
                      unsigned long *command_event_info_time_us,
                      stimulation_pulse_piece command_pieces[3],
                      unsigned char *command_event_type,
                      unsigned short *command_target_voltage);

void create_command(unsigned int event_id, unsigned char *command_channel,
                    unsigned short *command_event_info_event_id,
                    unsigned char *command_event_info_execution_condition,
                    unsigned long *command_event_info_time_us,
                    stimulation_pulse_piece command_pieces[3],
                    unsigned char *command_event_type,
                    unsigned short *command_target_voltage);

#endif
// End of code generation (create_command.h)
