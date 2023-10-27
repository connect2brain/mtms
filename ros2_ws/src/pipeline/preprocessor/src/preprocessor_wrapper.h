#ifndef PREPROCESSOR_WRAPPER_H
#define PREPROCESSOR_WRAPPER_H

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/preprocessed_eeg_sample.hpp"
#include "std_msgs/msg/string.hpp"

#include "ring_buffer.h"

namespace py = pybind11;

const size_t UNSET_SIZE = 0;

class PreprocessorWrapper {
public:
  PreprocessorWrapper(rclcpp::Logger logger);
  ~PreprocessorWrapper();

  void reset_module(const std::string& directory, const std::string& module_name);
  void initialize_arrays();

  std::pair<eeg_interfaces::msg::PreprocessedEegSample, bool> process(
    const RingBuffer<std::shared_ptr<eeg_interfaces::msg::EegSample>>& buffer,
    double_t current_time);

  bool is_initialized() const;
  std::size_t get_buffer_size() const;

  void set_eeg_data_size(size_t eeg_data_size);
  void set_emg_data_size(size_t emg_data_size);

private:
  rclcpp::Logger logger;

  bool _is_initialized;

  py::module preprocessor_module;
  py::object preprocessor_instance;

  std::unique_ptr<py::scoped_interpreter> guard;

  std::unique_ptr<py::array_t<double>> py_time;
  std::unique_ptr<py::array_t<double>> py_eeg_data;
  std::unique_ptr<py::array_t<double>> py_emg_data;

  int earliest_sample;
  int latest_sample;

  std::size_t buffer_size = UNSET_SIZE;
  std::size_t eeg_data_size = UNSET_SIZE;
  std::size_t emg_data_size = UNSET_SIZE;
};

#endif
