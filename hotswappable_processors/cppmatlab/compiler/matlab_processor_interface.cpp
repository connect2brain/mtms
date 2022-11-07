//
// Created by alqio on 8.9.2022.
//
#include "matlab_processor_interface.h"


MatlabProcessorInterface *MatlabProcessorInterface::init() {
  std::cout
      << "ERROR: in cpp_processor_interface init, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;

}

void MatlabProcessorInterface::init_experiment(coder::array<matlab_fpga_event, 1U> &ret) {
  std::cout
      << "ERROR: in cpp_processor_interface init_experiment, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;

}

void MatlabProcessorInterface::end_experiment(coder::array<matlab_fpga_event, 1U> &ret) {
  std::cout
      << "ERROR: in cpp_processor_interface end_experiment, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;
}

void MatlabProcessorInterface::data_received(const double channel_data_data[], int channel_data_size,
                                             unsigned long time_us,
                                             boolean_T first_sample_of_experiment,
                                             coder::array<matlab_fpga_event, 1U> &ret) {
  std::cout
      << "ERROR, in cpp_processor_interface data_received, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      <<
      std::endl;
}
