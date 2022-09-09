//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_CPP_PROCESSOR_H
#define DATA_PROCESSOR_CPP_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include "matlab_processor_interface.h"


class CPPProcessor : public ProcessorWrapper {
public:
  explicit CPPProcessor(const std::string& script_path);

  void init() override;

  std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data) override;

  int close() override;

private:
  void *processor_factory;
  MatlabProcessorInterface* inner_processor;
};


#endif //DATA_PROCESSOR_CPP_PROCESSOR_H
