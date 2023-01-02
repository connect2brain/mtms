//
// Created by alqio on 9/12/22.
//

#ifndef EEG_PROCESSOR_DATA_PROCESSOR_H
#define EEG_PROCESSOR_DATA_PROCESSOR_H

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "compiled_matlab_processor/compiled_matlab_processor.h"
#include "processor.h"

#include "mtms_interfaces/msg/event.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"

#include "fpga_interfaces/srv/send_pulse.hpp"
#include "fpga_interfaces/srv/send_charge.hpp"
#include "fpga_interfaces/srv/send_discharge.hpp"
#include "fpga_interfaces/srv/send_signal_out.hpp"
#include "fpga_interfaces/srv/start_experiment.hpp"
#include "fpga_interfaces/srv/stop_experiment.hpp"

#include "cpp_processor/cpp_processor.h"

#if defined(MATLAB_FOUND)

#include "matlab_processor/matlab_processor.h"

#endif

#if defined(PYTHON_FOUND)

#include "python_processor/python_processor.h"

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
  bool should_publish_events;
  void publish_events(double_t time, const std::vector<FpgaEvent> &events);
  void measure(int repeats);
  void send_fpga_events(const std::vector<FpgaEvent>& events);

  rclcpp::TimerBase::SharedPtr timer_;
  ProcessorWrapper *processor;
  std::fstream f;

  rclcpp::Publisher<mtms_interfaces::msg::Event>::SharedPtr event_publisher;

  rclcpp::Subscription<mtms_interfaces::msg::EegDatapoint>::SharedPtr eeg_data_subscription;

  rclcpp::Service<fpga_interfaces::srv::StartExperiment>::SharedPtr start_experiment_service;
  rclcpp::Service<fpga_interfaces::srv::StopExperiment>::SharedPtr stop_experiment_service;

  rclcpp::Client<fpga_interfaces::srv::StartExperiment>::SharedPtr start_experiment_client;
  std::shared_ptr<fpga_interfaces::srv::StartExperiment::Request> start_experiment_request;
  rclcpp::Client<fpga_interfaces::srv::StopExperiment>::SharedPtr stop_experiment_client;
  std::shared_ptr<fpga_interfaces::srv::StopExperiment::Request> stop_experiment_request;

  rclcpp::Client<fpga_interfaces::srv::SendPulse>::SharedPtr pulse_client;
  std::shared_ptr<fpga_interfaces::srv::SendPulse::Request> pulse_request;
  rclcpp::Client<fpga_interfaces::srv::SendCharge>::SharedPtr charge_client;
  std::shared_ptr<fpga_interfaces::srv::SendCharge::Request> charge_request;
  rclcpp::Client<fpga_interfaces::srv::SendDischarge>::SharedPtr discharge_client;
  std::shared_ptr<fpga_interfaces::srv::SendDischarge::Request> discharge_request;
  rclcpp::Client<fpga_interfaces::srv::SendSignalOut>::SharedPtr signal_out_client;
  std::shared_ptr<fpga_interfaces::srv::SendSignalOut::Request> signal_out_request;
};

#endif //EEG_PROCESSOR_EEG_PROCESSOR_H
