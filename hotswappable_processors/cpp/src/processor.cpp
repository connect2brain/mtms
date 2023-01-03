//
// Created by alqio on 11.11.2022.
//

#include <iostream>
#include "processor.h"


std::vector<fpga_event>
Processor::data_received(std::vector<double> channel_data, double time, bool first_sample_of_experiment) {
  std::cout << "in data received" << std::endl;

  this->samples.append(channel_data);

  std::vector<fpga_event> events;
  return events;
}

std::vector<fpga_event> Processor::end_experiment() {
  std::cout << "in end experiment" << std::endl;
  std::vector<fpga_event> events;
  return events;
}

std::vector<fpga_event> Processor::init_experiment() {
  std::cout << "in init experiment" << std::endl;

  std::vector<fpga_event> events;
  return events;
}

Processor::Processor() : samples(5, 2) {
  std::cout << "constructor" << std::endl;
}
