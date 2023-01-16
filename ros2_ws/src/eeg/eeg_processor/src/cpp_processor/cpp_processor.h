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

class CppProcessor : public ProcessorWrapper {
public:
  explicit CppProcessor(const std::string &script_path);

  std::vector<Event> init() override;

  std::vector<Event> data_received(mtms_interfaces::msg::EegDatapoint data) override;

  std::vector<Event> close() override;

private:
  void *processor_factory;
  std::unique_ptr<CppProcessorInterface> inner_processor;
};


#endif //PROCESSOR_FACTORY_CPP_PROCESSOR_H
