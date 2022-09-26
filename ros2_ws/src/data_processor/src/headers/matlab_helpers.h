//
// Created by alqio on 14.9.2022.
//

#ifndef DATA_PROCESSOR_MATLAB_HELPERS_H
#define DATA_PROCESSOR_MATLAB_HELPERS_H

#include "run_processor_types.h"
#include <iostream>
#include "fpga_event.h"

void print_matlab_fpga_event(matlab_fpga_event event);

FpgaEvent convert_matlab_fpga_event_to_fpga_event(matlab_fpga_event event);

#endif //DATA_PROCESSOR_MATLAB_HELPERS_H
