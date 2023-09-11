//
// Created by alqio on 8.9.2022.
//
#include "matlab_processor_interface.h"


MatlabProcessorInterface *MatlabProcessorInterface::init() {
  std::cout
      << "ERROR: in MatlabProcessorInterface init, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;

}

void MatlabProcessorInterface::init_session(coder::array<matlab_event, 1U> &ret) {
  std::cout
      << "ERROR: in MatlabProcessorInterface init_session, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;

}

void MatlabProcessorInterface::end_session(coder::array<matlab_event, 1U> &ret) {
  std::cout
      << "ERROR: in MatlabProcessorInterface end_session, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;
}

void MatlabProcessorInterface::data_received(const double channel_data_data[], int channel_data_size,
                                             double time,
                                             boolean_T first_sample_of_session,
                                             coder::array<matlab_event, 1U> &ret,
                                             coder::array<matlab_eeg_sample, 1U> &b_samples) {
  std::cout
      << "ERROR, in MatlabProcessorInterface data_received, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      <<
      std::endl;
}
