//
// Created by alqio on 14.9.2022.
//

#ifndef EEG_PROCESSOR_MATLAB_HELPERS_H
#define EEG_PROCESSOR_MATLAB_HELPERS_H

#include "run_processor_types.h"
#include <iostream>
#include "event.h"

void print_matlab_event(matlab_event event);

Event convert_matlab_event_to_event(matlab_event event);

#endif //EEG_PROCESSOR_MATLAB_HELPERS_H
