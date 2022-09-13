//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_MATLAB_PROCESSOR_H
#define DATA_PROCESSOR_MATLAB_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

class MatlabProcessor : public ProcessorWrapper {
public:
  MatlabProcessor(const std::string& script_path);

  void init();

  std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data);

  int close();

private:
  std::unique_ptr<matlab::engine::MATLABEngine> matlab;
  matlab::data::Array processor_instance;
};


#endif //DATA_PROCESSOR_MATLAB_PROCESSOR_H
