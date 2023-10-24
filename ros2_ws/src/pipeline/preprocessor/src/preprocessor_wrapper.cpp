#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "preprocessor_wrapper.h"
#include <eeg_interfaces/msg/eeg_sample.hpp>

namespace py = pybind11;

PreprocessorWrapper::PreprocessorWrapper() : _is_initialized(false) {
  guard = std::make_unique<py::scoped_interpreter>();
}

void PreprocessorWrapper::reset_module(const std::string& directory, const std::string& module_name) {
  preprocessor_module.release();
  preprocessor_instance.release();

  /* Set the sys.path to include the directory of the module. */
  py::module sys_module = py::module::import("sys");
  py::list sys_path = sys_module.attr("path");
  sys_path.append(directory);

  /* Import the module and initialize the Preprocessor instance. */
  preprocessor_module = py::module::import(module_name.c_str());
  preprocessor_instance = preprocessor_module.attr("Preprocessor")();

  _is_initialized = true;
}

bool PreprocessorWrapper::is_initialized() const {
    return _is_initialized;
}

eeg_interfaces::msg::PreprocessedEegSample PreprocessorWrapper::process_sample(const eeg_interfaces::msg::EegSample& sample) {
  /* TODO: Currently manually converts ROS message to Python dict and back. This should
    be done so that ROS messages would be directly exposed to the Python side. However,
    it was not easy to get it working. */
  py::dict py_sample;
  py_sample["time"] = sample.time;
  py_sample["eeg_data"] = sample.eeg_data;
  py_sample["emg_data"] = sample.emg_data;

  py::object result = preprocessor_instance.attr("process_sample")(py_sample);

  eeg_interfaces::msg::PreprocessedEegSample cpp_result;

  if (py::isinstance<py::dict>(result)) {
    py::dict dict_result = result.cast<py::dict>();
    cpp_result.time = dict_result["time"].cast<double>();
    cpp_result.eeg_data = dict_result["eeg_data"].cast<std::vector<double>>();
    cpp_result.emg_data = dict_result["emg_data"].cast<std::vector<double>>();
    cpp_result.valid = dict_result["valid"].cast<bool>();
  }
  return cpp_result;
}

PYBIND11_MODULE(my_module_wrapper, m) {
  py::class_<PreprocessorWrapper>(m, "PreprocessorWrapper")
    .def(py::init<>())
    .def("process_sample", &PreprocessorWrapper::process_sample);
}
