//
// Created by alqio on 8.9.2022.
//
#include "matlab_processor_interface.h"


MatlabProcessorInterface *MatlabProcessorInterface::init(unsigned int b_window_size, unsigned short channel_count) {
  std::cout
      << "ERROR: in cpp_processor_interface init, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;

}

void MatlabProcessorInterface::data_received(const double channel_data[62],
                                             coder::array<stimulation_pulse_event, 1U> &ret) {
  std::cout
      << "ERROR, in cpp_processor_interface data_received, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;
}