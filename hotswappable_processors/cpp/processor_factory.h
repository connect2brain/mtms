//
// Created by alqio on 8.9.2022.
//

#ifndef COMPILER_PROCESSOR_FACTORY_H
#define COMPILER_PROCESSOR_FACTORY_H

#include "processor.h"
#include <chrono>


extern "C" Processor *create_processor();

extern "C" void destroy_processor(Processor *p);

#endif //COMPILER_PROCESSOR_FACTORY_H
