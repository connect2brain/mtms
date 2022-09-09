//
// Created by alqio on 8.9.2022.
//

#ifndef COMPILER_PROCESSOR_FACTORY_H
#define COMPILER_PROCESSOR_FACTORY_H

#include "MatlabProcessorInterface.h"


extern "C" MatlabProcessorInterface *create_processor(unsigned int b_window_size,
                                                      unsigned short channel_count);

extern "C" void destroy(MatlabProcessorInterface *p);

#endif //COMPILER_PROCESSOR_FACTORY_H
