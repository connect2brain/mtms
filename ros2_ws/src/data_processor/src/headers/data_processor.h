//
// Created by alqio on 9/12/22.
//

#ifndef DATA_PROCESSOR_DATA_PROCESSOR_H
#define DATA_PROCESSOR_DATA_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "python_processor.h"
#include "matlab_processor.h"
#include "compiled_matlab_processor.h"
#include "processor.h"

#include <string>
#include <fstream>

double fRand(double fMin, double fMax) {
  double f = (double) rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

class DataProcessor : public rclcpp::Node {
public:
  DataProcessor();

  int shutdown();

private:
  void measure(int repeats);

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<mtms_interfaces::msg::EegDatapoint>::SharedPtr eeg_data_subscription;
  ProcessorWrapper *processor;
  std::fstream f;
};

#endif //DATA_PROCESSOR_DATA_PROCESSOR_H
