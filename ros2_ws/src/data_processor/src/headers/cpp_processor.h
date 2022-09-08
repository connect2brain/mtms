//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_CPP_PROCESSOR_H
#define DATA_PROCESSOR_CPP_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>


class CPPProcessor : public ProcessorWrapper {
public:
  CPPProcessor(std::string script_path);

  void init();

  std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data);

  int close();

private:
  void *processor_factory;
};


#endif //DATA_PROCESSOR_CPP_PROCESSOR_H
