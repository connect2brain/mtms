//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_MATLAB_PROCESSOR_H
#define DATA_PROCESSOR_MATLAB_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include "MatlabDataArray.hpp"
#include "MatlabDataArray/InputLayout.hpp"
#include "MatlabEngine.hpp"
#include "run_processor_types.h"
#include "matlab_helpers.h"

struct MatlabProcessorReturn {
  matlab::data::TypedArray<double> data;
  //matlab::data::TypedArray<matlab_fpga_event> events;
  std::vector<matlab_fpga_event> events;
};

class MatlabProcessor : public ProcessorWrapper {
public:
  MatlabProcessor(const std::string &script_path);

  std::vector<FpgaEvent> init();

  std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data);

  std::vector<FpgaEvent> close();

private:
  std::unique_ptr<matlab::engine::MATLABEngine> matlab;
  matlab::data::Array processor_instance;
  //matlab::data::TypedArray<double> matlab_data;
  //std::vector<std::vector<double> > matlab_data;
  std::vector<double> matlab_data;
  matlab::data::ArrayFactory factory;
};


#endif //DATA_PROCESSOR_MATLAB_PROCESSOR_H
