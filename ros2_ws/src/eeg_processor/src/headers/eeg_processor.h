//
// Created by alqio on 9/12/22.
//

#ifndef EEG_PROCESSOR_DATA_PROCESSOR_H
#define EEG_PROCESSOR_DATA_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "compiled_matlab_processor.h"
#include "processor.h"

#if defined(MATLAB_FOUND)

#include "matlab_processor.h"

#endif

#if defined(PYTHON_FOUND)

#include "python_processor.h"

#endif

#include <string>
#include <fstream>

double fRand(double fMin, double fMax) {
  double f = (double) rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

class EegProcessor : public rclcpp::Node {
public:
  EegProcessor();

  int shutdown();

private:
  void measure(int repeats);

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<mtms_interfaces::msg::EegDatapoint>::SharedPtr eeg_data_subscription;
  ProcessorWrapper *processor;
  std::fstream f;
};

#endif //EEG_PROCESSOR_EEG_PROCESSOR_H
