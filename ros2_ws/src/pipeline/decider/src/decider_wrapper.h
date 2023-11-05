#ifndef DECIDER_WRAPPER_H
#define DECIDER_WRAPPER_H

#include <memory>
#include <string>
#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include "rclcpp/rclcpp.hpp"

#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/preprocessed_eeg_sample.hpp"

#include "pipeline_interfaces/msg/sensory_stimulus.hpp"

#include "std_msgs/msg/string.hpp"

#include "ring_buffer.h"

namespace py = pybind11;

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

  void reset_module();

  std::tuple<bool, bool, std::shared_ptr<pipeline_interfaces::msg::SensoryStimulus>> process(
    const RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedEegSample>>& buffer,
    double_t sample_time,
    bool ready_for_event_trigger);

  bool is_initialized() const;
  std::size_t get_buffer_size() const;

  /* Exposed to Python, defined in cpp_bindings.cpp. */
  static void log(const std::string& message);

private:
  /* XXX: Have a static ROS2 logger to expose it more easily to the Python side (see cpp_bindings.cpp). */
  static rclcpp::Logger* logger_ptr;

  bool _is_initialized;

  py::module decider_module;
  py::object decider_instance;

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
