//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_PROCESSOR_H
#define DATA_PROCESSOR_PROCESSOR_H

#include "string"

class ProcessorWrapper {
public:
  virtual void init() = 0;

  virtual void data_received(int data) = 0;

  virtual int close() = 0;

};


#endif //DATA_PROCESSOR_PROCESSOR_H
