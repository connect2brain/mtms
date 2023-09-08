//
// Created by alqio on 11.11.2022.
//

#ifndef PROCESSOR_INTERFACE_H
#define PROCESSOR_INTERFACE_H

#include <vector>
#include "mtms_device_event.h"

class ProcessorInterface {
public:
  virtual ~ProcessorInterface() = default;

  virtual std::vector<mtms_device_event> init_session();

  virtual std::vector<mtms_device_event> end_session();

  virtual std::vector<mtms_device_event>
  data_received(std::vector<double> channel_data, double time, bool first_sample_of_session);

  virtual std::vector<eeg_sample>
  raw_eeg_received(std::vector<double> channel_data, double time, bool first_sample_of_session);
};

#endif //PROCESSOR_INTERFACE_H
