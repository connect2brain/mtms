//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_FACTORY_PROCESSOR_H
#define PROCESSOR_FACTORY_PROCESSOR_H

#include "processor_interface.h"

class Processor : public ProcessorInterface {
public:
  Processor();

  virtual std::vector<fpga_event> init_experiment();

  virtual std::vector<fpga_event> end_experiment();

  virtual std::vector<fpga_event>
  data_received(std::vector<double> channel_data, double time, bool first_sample_of_experiment);

};

#endif //PROCESSOR_FACTORY_PROCESSOR_H
