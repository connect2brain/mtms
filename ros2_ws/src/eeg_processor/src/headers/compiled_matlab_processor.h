//
// Created by alqio on 1.9.2022.
//

#ifndef EEG_PROCESSOR_CPP_PROCESSOR_H
#define EEG_PROCESSOR_CPP_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include "matlab_processor_interface.h"
#include "matlab_helpers.h"

class CompiledMatlabProcessor : public ProcessorWrapper {
public:
  explicit CompiledMatlabProcessor(const std::string &script_path);

  std::vector<FpgaEvent> init() override;

  std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data) override;

  std::vector<FpgaEvent> close() override;

private:
  void *processor_factory;
  std::unique_ptr<MatlabProcessorInterface> inner_processor;
};


#endif //EEG_PROCESSOR_CPP_PROCESSOR_H
