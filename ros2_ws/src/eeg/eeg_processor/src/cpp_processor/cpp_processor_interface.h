//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_FACTORY_CPP_PROCESSOR_INTERFACE_H
#define PROCESSOR_FACTORY_CPP_PROCESSOR_INTERFACE_H

#include <vector>
#include "run_processor_types.h"

class CppProcessorInterface {
public:
  virtual ~CppProcessorInterface() = default;

  virtual std::vector<matlab_event> init_experiment();

  virtual std::vector<matlab_event> end_experiment();

  virtual std::vector<matlab_event>
  data_received(std::vector<double> channel_data, double time, bool first_sample_of_experiment);

};

using create_cpp_processor = CppProcessorInterface *(*)();


#endif //PROCESSOR_FACTORY_CPP_PROCESSOR_INTERFACE_H
