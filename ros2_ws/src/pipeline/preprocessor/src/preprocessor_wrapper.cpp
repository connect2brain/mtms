#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

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

void PreprocessorWrapper::initialize_arrays(std::size_t buffer_size, std::size_t eeg_data_size, std::size_t emg_data_size) {
  py_time = std::make_unique<py::array_t<double>>(buffer_size);

  std::vector<size_t> eeg_data_shape = {buffer_size, eeg_data_size};
  py_eeg_data = std::make_unique<py::array_t<double>>(eeg_data_shape);

  std::vector<size_t> emg_data_shape = {buffer_size, emg_data_size};
  py_emg_data = std::make_unique<py::array_t<double>>(emg_data_shape);

  this->buffer_size = buffer_size;
  this->eeg_data_size = eeg_data_size;
  this->emg_data_size = emg_data_size;
}

PreprocessorWrapper::~PreprocessorWrapper() {
    py_time.reset();
    py_eeg_data.reset();
    py_emg_data.reset();
}

bool PreprocessorWrapper::is_initialized() const {
    return _is_initialized;
}

eeg_interfaces::msg::PreprocessedEegSample
PreprocessorWrapper::process_sample_buffer(const RingBuffer<std::shared_ptr<eeg_interfaces::msg::EegSample>>& buffer) {

  /* Fill the numpy arrays. */
  auto time_ptr = py_time->mutable_data();
  auto eeg_data_ptr = py_eeg_data->mutable_data();
  auto emg_data_ptr = py_emg_data->mutable_data();

  for (const auto& sample_ptr : buffer.get_buffer()) {
    const auto& sample = *sample_ptr;

    *time_ptr++ = sample.time;
    std::memcpy(eeg_data_ptr, sample.eeg_data.data(), eeg_data_size * sizeof(double));
    eeg_data_ptr += eeg_data_size;
    std::memcpy(emg_data_ptr, sample.emg_data.data(), emg_data_size * sizeof(double));
    emg_data_ptr += emg_data_size;
  }

  /* Call the Python function. */
  py::object result = preprocessor_instance.attr("process_sample_buffer")(*py_time, *py_eeg_data, *py_emg_data);

  /* Convert the Python dictionary to a ROS message. */
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
    .def("process_sample_buffer", &PreprocessorWrapper::process_sample_buffer);
}
