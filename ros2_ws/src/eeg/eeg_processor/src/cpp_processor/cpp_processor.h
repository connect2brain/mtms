//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_FACTORY_CPP_PROCESSOR_H
#define PROCESSOR_FACTORY_CPP_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include "cpp_processor_interface.h"

template<class InputType, class OutputType>
class CppProcessor : public ProcessorWrapper<InputType, OutputType> {
public:
  explicit CppProcessor(const std::string &script_path);

  std::vector<OutputType> init() override;

  std::vector<OutputType> eeg_received(mtms_interfaces::msg::EegDatapoint sample) override;

  std::vector<OutputType> event_received(mtms_interfaces::msg::Event event) override;

  void close();

private:
  void *processor_factory;
  std::unique_ptr<CppProcessorInterface> inner_processor;
};


#endif //PROCESSOR_FACTORY_CPP_PROCESSOR_H
