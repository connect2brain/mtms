//
// Created by alqio on 11.11.2022.
//

#include <iostream>
#include "processor.h"
#include "utils.h"

std::vector<eeg_sample>
Processor::raw_eeg_received(std::vector<double> channel_data, double time, bool first_sample_of_session) {
  std::vector<eeg_sample> cleaned_samples;

  eeg_sample sample;
  sample.channel_data = channel_data;
  sample.time = time;
  sample.first_sample_of_session = first_sample_of_session;

  cleaned_samples.push_back(sample);

  return cleaned_samples;

}

std::vector<mtms_device_event>
Processor::data_received(std::vector<double> channel_data, double time, bool first_sample_of_session) {
  this->samples.append(channel_data);
  this->samples_collected++;

  std::vector<mtms_device_event> events;

  if (this->samples.full && this->samples_collected % this->isi_samples == 0) {
    auto buf = &this->samples.buffer;
    auto event = create_trigger_out_command(1, 1, 1000, 2, time);
    events.push_back(event);
    this->samples_collected = 0;
  }

  return events;
}

std::vector<mtms_device_event> Processor::end_session() {
  std::cout << "in end session" << std::endl;
  std::vector<mtms_device_event> events;
  return events;
}

std::vector<mtms_device_event> Processor::init_session() {
  std::cout << "in init session" << std::endl;

  std::vector<mtms_device_event> events;
  return events;
}

Processor::Processor() : samples(5000, 62) {
  this->sampling_frequency = 5000;
  this->window_size = 5000;
  this->channel_count = 62;

  this->isi_seconds = 1;
  this->isi_samples = 100;

  this->samples_collected = 0;

  std::cout << "constructor" << std::endl;
}
