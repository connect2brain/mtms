#ifndef PREPROCESSOR_WRAPPER_H
#define PREPROCESSOR_WRAPPER_H

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <memory>
#include <string>

#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/preprocessed_eeg_sample.hpp"
#include "std_msgs/msg/string.hpp"

namespace py = pybind11;

class PreprocessorWrapper {
public:
  PreprocessorWrapper();
  void reset_module(const std::string& directory, const std::string& module_name);
  eeg_interfaces::msg::PreprocessedEegSample process_sample(const eeg_interfaces::msg::EegSample& sample);
  bool is_initialized() const;

private:
  bool _is_initialized;

  py::module preprocessor_module;
  py::object preprocessor_instance;

  std::unique_ptr<py::scoped_interpreter> guard;
};

#endif
