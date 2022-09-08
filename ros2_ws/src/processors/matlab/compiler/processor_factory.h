//
// Created by alqio on 8.9.2022.
//

#ifndef COMPILER_PROCESSOR_FACTORY_H
#define COMPILER_PROCESSOR_FACTORY_H

#include "Processor.h"


extern "C" Processor *create_processor();

extern "C" void destroy(Processor *p);

#endif //COMPILER_PROCESSOR_FACTORY_H
