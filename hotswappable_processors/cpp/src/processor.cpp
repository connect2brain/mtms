//
// Created by alqio on 11.11.2022.
//

#include <iostream>
#include "processor.h"
#include "utils.h"

std::vector<fpga_event>
Processor::data_received(std::vector<double> channel_data, double time, bool first_sample_of_experiment) {
  //std::cout << "in data received" << std::endl;

  this->samples.append(channel_data);
  this->samples_collected++;

  std::vector<fpga_event> events;

  if (this->samples_collected % this->isi_samples == 0) {
    auto buf = this->samples.get_buffer();
    auto event = create_signal_out_command(1, 1, 1000, 2, 5.0);
    events.push_back(event);
    this->samples_collected = 0;
  }

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

Processor::Processor() : samples(5000, 62) {
  this->sampling_frequency = 5000;
  this->window_size = 5000;
  this->channel_count = 62;

  this->isi_seconds = 1;
  this->isi_samples = this->isi_seconds * this->sampling_frequency;

  this->samples_collected = 0;

  std::cout << "constructor" << std::endl;
}
