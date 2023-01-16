//
// Created by alqio on 1.9.2022.
//

#ifndef EEG_PROCESSOR_MATLAB_PROCESSOR_H
#define EEG_PROCESSOR_MATLAB_PROCESSOR_H

#include "processor.h"
#include "iostream"
#include "MatlabDataArray.hpp"
#include "MatlabDataArray/InputLayout.hpp"
#include "MatlabEngine.hpp"
#include "run_processor_types.h"
#include "matlab_helpers.h"

class MatlabProcessor : public ProcessorWrapper {
public:
  MatlabProcessor(const std::string &script_path);

  std::vector<Event> init();

  std::vector<Event> data_received(mtms_interfaces::msg::EegDatapoint data);

  std::vector<Event> close();

private:
  std::unique_ptr<matlab::engine::MATLABEngine> matlab;
  matlab::data::Array processor_instance;
  std::vector<double> matlab_data;
  matlab::data::ArrayFactory factory;
};


#endif //EEG_PROCESSOR_MATLAB_PROCESSOR_H
