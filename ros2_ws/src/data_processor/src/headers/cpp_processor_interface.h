//
// Created by alqio on 8.9.2022.
//

#ifndef DATA_PROCESSOR_CPP_PROCESSOR_INTERFACE_H
#define DATA_PROCESSOR_CPP_PROCESSOR_INTERFACE_H

#include "coder_array.h"
#include <iostream>


class MatlabProcessorInterface {
public:
  MatlabProcessorInterface *init(unsigned int b_window_size,
                                 unsigned short channel_count);
  void data_received(const double channel_data[62]);
  coder::array<double, 2U> data;
  unsigned int window_size;
};


typedef MatlabProcessorInterface *create_processor();

typedef void destroy(MatlabProcessorInterface *);

#endif //DATA_PROCESSOR_CPP_PROCESSOR_INTERFACE_H
