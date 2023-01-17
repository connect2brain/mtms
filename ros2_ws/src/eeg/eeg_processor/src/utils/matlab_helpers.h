//
// Created by alqio on 14.9.2022.
//

#ifndef EEG_PROCESSOR_MATLAB_HELPERS_H
#define EEG_PROCESSOR_MATLAB_HELPERS_H

#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "run_processor_types.h"
#include <iostream>
#include "event.h"

void print_matlab_event(matlab_event event);

Event convert_matlab_event_to_event(matlab_event event);
mtms_interfaces::msg::EegDatapoint convert_matlab_sample_to_sample(matlab_eeg_sample matlab_sample);

#endif //EEG_PROCESSOR_MATLAB_HELPERS_H
