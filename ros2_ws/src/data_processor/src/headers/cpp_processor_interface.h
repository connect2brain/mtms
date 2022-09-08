//
// Created by alqio on 8.9.2022.
//

#ifndef DATA_PROCESSOR_CPP_PROCESSOR_INTERFACE_H
#define DATA_PROCESSOR_CPP_PROCESSOR_INTERFACE_H

#include "coder_array.h"

class CppProcessorInterface {
public:
  CppProcessorInterface *init(unsigned int b_window_size);

  void enqueue(const double element[62]);

  void data_received(const double channel_data[62]);

  coder::array<double, 2U> data;
  unsigned int window_size;
};

typedef CppProcessorInterface *create_processor();

typedef void destroy(CppProcessorInterface *);

#endif //DATA_PROCESSOR_CPP_PROCESSOR_INTERFACE_H
