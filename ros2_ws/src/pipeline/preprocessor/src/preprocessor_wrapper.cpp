#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "preprocessor_wrapper.h"
#include <eeg_interfaces/msg/eeg_sample.hpp>

PreprocessorWrapper::PreprocessorWrapper(rclcpp::Logger& logger) {
  logger_ptr = &logger;
  _is_initialized = false;
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

  /* Extract the sample_window from preprocessor_instance. */
  if (py::hasattr(preprocessor_instance, "sample_window")) {
    py::list sample_window = preprocessor_instance.attr("sample_window").cast<py::list>();
    if (sample_window.size() == 2) {
      this->earliest_sample = sample_window[0].cast<int>();
      this->latest_sample = sample_window[1].cast<int>();

      this->buffer_size = latest_sample - earliest_sample + 1;
    } else {
      RCLCPP_WARN(*logger_ptr, "sample_window class attribute is of incorrect length (should be two elements).");
    }
  } else {
    RCLCPP_WARN(*logger_ptr, "sample_window class attribute not defined by the preprocessor.");
  }

  RCLCPP_INFO(*logger_ptr, "Preprocessor set to: %s.", module_name.c_str());
  RCLCPP_INFO(*logger_ptr, " ");
  RCLCPP_INFO(*logger_ptr, "Preprocessor configuration");
  RCLCPP_INFO(*logger_ptr, " ");
  RCLCPP_INFO(*logger_ptr, "  - Sample window: [%d, %d]", this->earliest_sample, this->latest_sample);
  RCLCPP_INFO(*logger_ptr, " ");

  _is_initialized = true;
}

void PreprocessorWrapper::initialize_arrays() {
  if (this->buffer_size != UNSET_SIZE &&
      this->eeg_data_size != UNSET_SIZE &&
      this->emg_data_size != UNSET_SIZE) {

    py_time = std::make_unique<py::array_t<double>>(buffer_size);

    std::vector<size_t> eeg_data_shape = {buffer_size, eeg_data_size};
    py_eeg_data = std::make_unique<py::array_t<double>>(eeg_data_shape);

    std::vector<size_t> emg_data_shape = {buffer_size, emg_data_size};
    py_emg_data = std::make_unique<py::array_t<double>>(emg_data_shape);

    RCLCPP_DEBUG(*logger_ptr, "Arrays initialized for time, EEG data, and EMG data.");
  }
}

PreprocessorWrapper::~PreprocessorWrapper() {
  py_time.reset();
  py_eeg_data.reset();
  py_emg_data.reset();
}

bool PreprocessorWrapper::is_initialized() const {
  return this->_is_initialized;
}

std::size_t PreprocessorWrapper::get_buffer_size() const {
  return this->buffer_size;
}

void PreprocessorWrapper::set_eeg_data_size(size_t eeg_data_size) {
  this->eeg_data_size = eeg_data_size;
}

void PreprocessorWrapper::set_emg_data_size(size_t emg_data_size) {
  this->emg_data_size = emg_data_size;
}

std::pair<eeg_interfaces::msg::PreprocessedEegSample, bool>
PreprocessorWrapper::process(
    const RingBuffer<std::shared_ptr<eeg_interfaces::msg::EegSample>>& buffer,
    double_t current_time) {

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
  eeg_interfaces::msg::PreprocessedEegSample cpp_result;

  py::object result;
  try {
    result = preprocessor_instance.attr("process")(*py_time, *py_eeg_data, *py_emg_data);

  } catch(const py::error_already_set& e) {
    RCLCPP_ERROR(*logger_ptr, "Python error: %s", e.what());
    return {cpp_result, false};

  } catch(const std::exception& e) {
    RCLCPP_ERROR(*logger_ptr, "C++ error: %s", e.what());
    return {cpp_result, false};
  }

  /* Validate the return value of the Python function call. */
  if (!py::isinstance<py::dict>(result)) {
    RCLCPP_ERROR(*logger_ptr, "Python module should return a dictionary.");
    return {cpp_result, false};
  }

  py::dict dict_result = result.cast<py::dict>();

  if (!dict_result.contains("eeg_data")) {
    RCLCPP_ERROR(*logger_ptr, "Python module should return a dictionary with the field: eeg_data.");
    return {cpp_result, false};
  }
  if (!dict_result.contains("emg_data")) {
    RCLCPP_ERROR(*logger_ptr, "Python module should return a dictionary with the field: emg_data.");
    return {cpp_result, false};
  }
  if (!dict_result.contains("valid")) {
    RCLCPP_ERROR(*logger_ptr, "Python module should return a dictionary with the field: valid.");
    return {cpp_result, false};
  }

  /* Convert the Python dictionary to a ROS message. */
  cpp_result.eeg_data = dict_result["eeg_data"].cast<std::vector<double>>();
  cpp_result.emg_data = dict_result["emg_data"].cast<std::vector<double>>();
  cpp_result.valid = dict_result["valid"].cast<bool>();

  cpp_result.time = current_time;

  return {cpp_result, true};
}

rclcpp::Logger* PreprocessorWrapper::logger_ptr = nullptr;
