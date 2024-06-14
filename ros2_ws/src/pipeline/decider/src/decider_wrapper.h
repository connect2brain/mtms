#ifndef DECIDER_WRAPPER_H
#define DECIDER_WRAPPER_H

#include <memory>
#include <string>
#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include "rclcpp/rclcpp.hpp"

#include "eeg_interfaces/msg/sample.hpp"
#include "eeg_interfaces/msg/preprocessed_sample.hpp"
#include "experiment_interfaces/msg/trial.hpp"
#include "targeting_interfaces/msg/electric_target.hpp"
#include "pipeline_interfaces/msg/sensory_stimulus.hpp"

#include "std_msgs/msg/string.hpp"

#include "ring_buffer.h"

namespace py = pybind11;

enum class WrapperState {
  UNINITIALIZED,
  READY,
  ERROR
};

class DeciderWrapper {
public:
  DeciderWrapper(rclcpp::Logger& logger);
  ~DeciderWrapper();

  void initialize_module(
      const std::string& directory,
      const std::string& module_name,
      const size_t eeg_data_size,
      const size_t emg_data_size,
      const uint16_t sampling_frequency);

  void reset_module_state();

  std::vector<std::vector<targeting_interfaces::msg::ElectricTarget>> get_targets();

  std::tuple<bool, std::shared_ptr<experiment_interfaces::msg::Trial>, bool, bool> process(
    pipeline_interfaces::msg::SensoryStimulus& output_sensory_stimulus,
    const RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedSample>>& buffer,
    double_t sample_time,
    bool ready_for_trial);

  WrapperState get_state() const;

  std::size_t get_buffer_size() const;

  /* Exposed to Python, defined in cpp_bindings.cpp. */
  static void log(const std::string& message);

  /* Exposed to Python, defined in cpp_bindings.cpp. */
  static void log_throttle(const std::string& message, const double_t period);

private:
  /* XXX: Have a static ROS2 logger to expose it more easily to the Python side (see cpp_bindings.cpp). */
  static rclcpp::Logger* logger_ptr;

  WrapperState state;

  std::unique_ptr<py::module> decider_module;
  std::unique_ptr<py::object> decider_instance;

  std::unique_ptr<py::scoped_interpreter> guard;

  std::unique_ptr<py::array_t<double>> py_timestamps;
  std::unique_ptr<py::array_t<bool>> py_valid;
  std::unique_ptr<py::array_t<double>> py_eeg_data;
  std::unique_ptr<py::array_t<double>> py_emg_data;

  int earliest_sample;
  int latest_sample;
  uint16_t sampling_frequency;

  std::size_t buffer_size = 0;
  std::size_t eeg_data_size;
  std::size_t emg_data_size;
};

#endif
