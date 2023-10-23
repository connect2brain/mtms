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

  ~MatlabProcessor();

  std::vector<Event> init();

  std::vector<eeg_interfaces::msg::EegSample> raw_eeg_received(eeg_interfaces::msg::EegSample sample);

  std::vector<Event> cleaned_eeg_received(eeg_interfaces::msg::EegSample sample);

  std::vector<Event> present_stimulus_received(event_interfaces::msg::Stimulus event);

  std::vector<Event> end_session();

private:
  std::unique_ptr<matlab::engine::MATLABEngine> matlab;
  matlab::data::Array processor_instance;
  std::vector<double> matlab_data;
  matlab::data::ArrayFactory factory;
};


#endif //EEG_PROCESSOR_MATLAB_PROCESSOR_H
