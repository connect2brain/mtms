//
// Created by alqio on 11.11.2022.
//

#include "processor_interface.h"

#include <vector>
#include <iostream>

std::vector<mtms_device_event> ProcessorInterface::init_session() {
  std::cout
      << "ERROR: in cpp_processor_interface init_session, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;
}

std::vector<mtms_device_event> ProcessorInterface::end_session() {
  std::cout
      << "ERROR: in cpp_processor_interface end_session, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      << std::endl;
}

std::vector<mtms_device_event>
ProcessorInterface::data_received(std::vector<double> channel_data, double time, bool first_sample_of_session) {
  std::cout
      << "ERROR, in cpp_processor_interface data_received, we should not be here. Are the methods in MatlabProcessorInterface.h virtual?"
      <<
      std::endl;
}

std::vector<eeg_sample>
ProcessorInterface::raw_eeg_received(std::vector<double> channel_data, double time, bool first_sample_of_session) {
  std::cout << "ERROR, in cpp_processor_interface raw_eeg_received" << std::endl;
}
