//
// Created by alqio on 1.9.2022.
//

#ifndef EEG_PROCESSOR_PROCESSOR_H
#define EEG_PROCESSOR_PROCESSOR_H

#include "string"
#include "eeg_interfaces/msg/eeg_datapoint.hpp"
#include "event_interfaces/msg/stimulus.hpp"

#include "event.h"

class ProcessorWrapper {
public:
  virtual std::vector<Event> init() = 0;

  virtual std::vector<eeg_interfaces::msg::EegDatapoint>
  raw_eeg_received(eeg_interfaces::msg::EegDatapoint sample) = 0;

  virtual std::vector<Event> cleaned_eeg_received(eeg_interfaces::msg::EegDatapoint sample) = 0;

  virtual std::vector<Event> present_stimulus_received(event_interfaces::msg::Stimulus event) = 0;

};

#endif //EEG_PROCESSOR_PROCESSOR_H
