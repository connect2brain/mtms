//
// Created by alqio on 8.9.2022.
//

#ifndef COMPILER_PROCESSOR_FACTORY_H
#define COMPILER_PROCESSOR_FACTORY_H

#include "MatlabProcessor.h"
#include <chrono>


extern "C" MatlabProcessor *create_processor(unsigned int b_window_size,
                                             unsigned short channel_count);

extern "C" void destroy_processor(MatlabProcessor *p);

#endif //COMPILER_PROCESSOR_FACTORY_H
