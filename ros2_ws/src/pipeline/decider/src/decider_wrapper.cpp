#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "decider_wrapper.h"
#include <eeg_interfaces/msg/sample.hpp>

namespace py = pybind11;

DeciderWrapper::DeciderWrapper(rclcpp::Logger& logger) {
  logger_ptr = &logger;
  state = WrapperState::UNINITIALIZED;
  guard = std::make_unique<py::scoped_interpreter>();
}

void DeciderWrapper::initialize_module(
    const std::string& directory,
    const std::string& module_name,
    const size_t eeg_data_size,
    const size_t emg_data_size,
    const uint16_t sampling_frequency) {

  this->sampling_frequency = sampling_frequency;

  /* If we have an existing decider instance, release it which will call the destructor */
  decider_instance = nullptr;
  decider_module = nullptr;

  /* Set the sys.path to include the directory of the module. */
  py::module sys_module = py::module::import("sys");
  py::list sys_path = sys_module.attr("path");
  sys_path.append(directory);

  /* Remove the module from sys.modules if it exists, to ensure it is reloaded. */
  py::dict sys_modules = sys_module.attr("modules");
  if (sys_modules.contains(module_name.c_str())) {
    sys_modules.attr("__delitem__")(module_name.c_str());
  }

  /* Import the module and initialize the Decider instance. */

  try {
    auto imported_module = py::module::import(module_name.c_str());
    decider_module = std::make_unique<py::module>(imported_module);
    auto instance = decider_module->attr("Decider")(eeg_data_size, emg_data_size, sampling_frequency);
    decider_instance = std::make_unique<py::object>(instance);

  } catch(const py::error_already_set& e) {
    RCLCPP_ERROR(*logger_ptr, "Python error: %s", e.what());
    state = WrapperState::ERROR;
    return;

  } catch(const std::exception& e) {
    RCLCPP_ERROR(*logger_ptr, "C++ error: %s", e.what());
    state = WrapperState::ERROR;
    return;
  }

  /* Extract the sample_window from decider_instance. */
  if (py::hasattr(*decider_instance, "sample_window")) {
    py::list sample_window = decider_instance->attr("sample_window").cast<py::list>();
    if (sample_window.size() == 2) {
      this->earliest_sample = sample_window[0].cast<int>();
      this->latest_sample = sample_window[1].cast<int>();

      this->buffer_size = this->latest_sample - this->earliest_sample + 1;
    } else {
      RCLCPP_WARN(*logger_ptr, "sample_window class attribute is of incorrect length (should be two elements).");
    }
  } else {
    RCLCPP_WARN(*logger_ptr, "sample_window class attribute not defined by the decider.");
  }

  RCLCPP_INFO(*logger_ptr, "Decider set to: %s.", module_name.c_str());
  RCLCPP_INFO(*logger_ptr, " ");
  RCLCPP_INFO(*logger_ptr, "Decider configuration");
  RCLCPP_INFO(*logger_ptr, " ");
  RCLCPP_INFO(*logger_ptr, "  - Sample window: [%d, %d]", this->earliest_sample, this->latest_sample);
  RCLCPP_INFO(*logger_ptr, " ");

  /* Initialize numpy arrays. */
  py_timestamps = std::make_unique<py::array_t<double>>(buffer_size);

  py_valid = std::make_unique<py::array_t<bool>>(buffer_size);

  std::vector<size_t> eeg_data_shape = {buffer_size, eeg_data_size};
  py_eeg_data = std::make_unique<py::array_t<double>>(eeg_data_shape);

  std::vector<size_t> emg_data_shape = {buffer_size, emg_data_size};
  py_emg_data = std::make_unique<py::array_t<double>>(emg_data_shape);

  this->eeg_data_size = eeg_data_size;
  this->emg_data_size = emg_data_size;

  state = WrapperState::READY;
}

void DeciderWrapper::reset_module_state() {
  decider_instance = nullptr;
  decider_module = nullptr;

  py_timestamps.reset();
  py_eeg_data.reset();
  py_emg_data.reset();

  state = WrapperState::UNINITIALIZED;

  RCLCPP_INFO(*logger_ptr, "Decider reset.");
}

DeciderWrapper::~DeciderWrapper() {
  py_timestamps.reset();
  py_valid.reset();
  py_eeg_data.reset();
  py_emg_data.reset();
}

std::vector<std::vector<targeting_interfaces::msg::ElectricTarget>> DeciderWrapper::get_targets() {
  std::vector<std::vector<targeting_interfaces::msg::ElectricTarget>> targets;

  if (state != WrapperState::READY) {
    RCLCPP_ERROR(*logger_ptr, "Decider not ready.");
    return targets;
  }

  try {
    py::list py_targets = decider_instance->attr("targets").cast<py::list>();

    for (const auto& py_target : py_targets) {
      std::vector<targeting_interfaces::msg::ElectricTarget> target;
      py::list py_target_list = py_target.cast<py::list>();

      for (const auto& py_target_item : py_target_list) {
        py::dict py_target_dict = py_target_item.cast<py::dict>();

        targeting_interfaces::msg::ElectricTarget electric_target;
        electric_target.displacement_x = py_target_dict["displacement_x"].cast<uint8_t>();
        electric_target.displacement_y = py_target_dict["displacement_y"].cast<uint8_t>();
        electric_target.rotation_angle = py_target_dict["rotation_angle"].cast<uint16_t>();
        electric_target.intensity = py_target_dict["intensity"].cast<uint8_t>();

        std::string algorithm = py_target_dict["algorithm"].cast<std::string>();
        if (algorithm == "least_squares") {
          electric_target.algorithm.value = targeting_interfaces::msg::TargetingAlgorithm::LEAST_SQUARES;
        } else if (algorithm == "genetic") {
          electric_target.algorithm.value = targeting_interfaces::msg::TargetingAlgorithm::GENETIC;
        } else {
          RCLCPP_WARN(*logger_ptr, "Unknown targeting algorithm: %s, defaulting to 'least squares'.", algorithm.c_str());
          electric_target.algorithm.value = targeting_interfaces::msg::TargetingAlgorithm::LEAST_SQUARES;
        }

        target.push_back(electric_target);
      }
      targets.push_back(target);
    }

  } catch(const py::error_already_set& e) {
    RCLCPP_ERROR(*logger_ptr, "Python error: %s", e.what());
    state = WrapperState::ERROR;
    return targets;

  } catch(const std::exception& e) {
    RCLCPP_ERROR(*logger_ptr, "C++ error: %s", e.what());
    state = WrapperState::ERROR;
    return targets;
  }
  return targets;
}

WrapperState DeciderWrapper::get_state() const {
  return this->state;
}

std::size_t DeciderWrapper::get_buffer_size() const {
  return this->buffer_size;
}

std::tuple<bool, std::shared_ptr<experiment_interfaces::msg::Trial>, bool, bool> DeciderWrapper::process(
    pipeline_interfaces::msg::SensoryStimulus& output_sensory_stimulus,
    const RingBuffer<std::shared_ptr<eeg_interfaces::msg::PreprocessedSample>>& buffer,
    double_t sample_time,
    bool ready_for_trial,
    bool trigger) {

  bool success = true;
  std::shared_ptr<experiment_interfaces::msg::Trial> trial = nullptr;
  bool trigger_labjack = false;
  bool request_sensory_stimulus = false;

  /* TODO: The logic below, as well as the difference in semantics between "sample time" and "current time", needs to
     be documented somewhere more thoroughly. */

  /* An example: If the sample window is set to [-2, 2], the earliest sample will be -2, and the current sample,
     i.e., the sample corresponding to 0, will have the index 2, hence the calculation below. */
  int current_sample_index = -this->earliest_sample;

  /* An example: If sample time is 5.0 and the sampling frequency is 5 kHz, and we have a sample window [-2, 2]
     (the latest sample being 2), the sample 2 in the sample window corresponds to the time 5.0. Hence, sample 0
     corresponds to the time 5.0 s - 2 / (5000 Hz) = 4.9996 s. */
  double_t current_time = sample_time - (double)this->latest_sample / this->sampling_frequency;

  /* Fill the numpy arrays. */
  auto timestamps_ptr = py_timestamps->mutable_data();
  auto valid_ptr = py_valid->mutable_data();
  auto eeg_data_ptr = py_eeg_data->mutable_data();
  auto emg_data_ptr = py_emg_data->mutable_data();

  buffer.process_elements([&](const auto& sample_ptr) {
    const auto& sample = *sample_ptr;

    *timestamps_ptr++ = sample.time - current_time;
    *valid_ptr++ = sample.valid;
    std::memcpy(eeg_data_ptr, sample.eeg_data.data(), eeg_data_size * sizeof(double));
    eeg_data_ptr += eeg_data_size;
    std::memcpy(emg_data_ptr, sample.emg_data.data(), emg_data_size * sizeof(double));
    emg_data_ptr += emg_data_size;
  });

  /* Call the Python function. */
  py::object result;
  try {
    result = decider_instance->attr("process")(current_time, *py_timestamps, *py_valid, *py_eeg_data, *py_emg_data, current_sample_index, ready_for_trial, trigger);

  } catch(const py::error_already_set& e) {
    RCLCPP_ERROR(*logger_ptr, "Python error: %s", e.what());
    state = WrapperState::ERROR;
    success = false;

    return {success, trial, trigger_labjack, request_sensory_stimulus};

  } catch(const std::exception& e) {
    RCLCPP_ERROR(*logger_ptr, "C++ error: %s", e.what());
    state = WrapperState::ERROR;
    success = false;

    return {success, trial, trigger_labjack, request_sensory_stimulus};
  }

  /* If the return value is None, return early but mark it as successful. */
  if (result.is_none()) {
    return {success, trial, trigger_labjack, request_sensory_stimulus};
  }

  /* If the return value is not None, ensure that it is a dictionary. */
  if (!py::isinstance<py::dict>(result)) {
    RCLCPP_ERROR(*logger_ptr, "Python module should return a dictionary.");
    state = WrapperState::ERROR;
    success = false;

    return {success, trial, trigger_labjack, request_sensory_stimulus};
  }

  py::dict dict_result = result.cast<py::dict>();

  if (dict_result.contains("trial")) {
    /* If there is a trial in the dictionary, extract it and return early. */
    py::dict py_trial = dict_result["trial"].cast<py::dict>();

    trial = std::make_shared<experiment_interfaces::msg::Trial>();

    /* Extract the targets from the dictionary. */
    if (!py_trial.contains("targets")) {
      RCLCPP_ERROR(*logger_ptr, "Trial dictionary does not contain the field: targets.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    py::list py_targets = py_trial["targets"].cast<py::list>();
    for (const auto& py_target : py_targets) {
      targeting_interfaces::msg::ElectricTarget electric_target;
      py::dict py_target_dict = py_target.cast<py::dict>();

      electric_target.displacement_x = py_target_dict["displacement_x"].cast<uint8_t>();
      electric_target.displacement_y = py_target_dict["displacement_y"].cast<uint8_t>();
      electric_target.rotation_angle = py_target_dict["rotation_angle"].cast<uint16_t>();
      electric_target.intensity = py_target_dict["intensity"].cast<uint8_t>();

      std::string algorithm = py_target_dict["algorithm"].cast<std::string>();
      if (algorithm == "least_squares") {
        electric_target.algorithm.value = targeting_interfaces::msg::TargetingAlgorithm::LEAST_SQUARES;
      } else if (algorithm == "genetic") {
        electric_target.algorithm.value = targeting_interfaces::msg::TargetingAlgorithm::GENETIC;
      } else {
        RCLCPP_WARN(*logger_ptr, "Unknown targeting algorithm: %s, defaulting to 'least squares'.", algorithm.c_str());
        electric_target.algorithm.value = targeting_interfaces::msg::TargetingAlgorithm::LEAST_SQUARES;
      }

      trial->targets.push_back(electric_target);
    }

    /* Extract the pulse times from the dictionary. */
    if (!py_trial.contains("pulse_times")) {
      RCLCPP_ERROR(*logger_ptr, "Trial dictionary does not contain the field: pulse_times.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    py::list py_pulse_times = py_trial["pulse_times"].cast<py::list>();
    auto first_pulse_time = py_pulse_times[0].cast<double_t>();
    for (const auto& py_pulse_time : py_pulse_times) {
      trial->pulse_times_since_trial_start.push_back(py_pulse_time.cast<double_t>() - first_pulse_time);
    }

    trial->timing.desired_start_time = first_pulse_time;
    trial->timing.allow_late = false;

    /* Extract the triggers from the dictionary. */
    if (!py_trial.contains("triggers")) {
      RCLCPP_ERROR(*logger_ptr, "Trial dictionary does not contain the field: triggers.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    py::list py_triggers = py_trial["triggers"].cast<py::list>();
    for (const auto& py_trigger : py_triggers) {
      experiment_interfaces::msg::TriggerConfig trigger;
      py::dict py_trigger_dict = py_trigger.cast<py::dict>();

      trigger.enabled = py_trigger_dict["enabled"].cast<bool>();
      trigger.delay = py_trigger_dict["delay"].cast<double_t>();

      trial->triggers.push_back(trigger);
    }

    trial->config.voltage_tolerance_proportion_for_precharging = 0.03;
    trial->config.use_pulse_width_modulation_approximation = true;
    trial->config.recharge_after_trial = true;
    trial->config.dry_run = false;
  }

  if (dict_result.contains("trigger_labjack")) {
    trigger_labjack = dict_result["trigger_labjack"].cast<bool>();
  }

  if (dict_result.contains("sensory_stimulus")) {
    py::dict py_sensory_stimulus = dict_result["sensory_stimulus"].cast<py::dict>();

    /* Checks for each field in the sensory_stimulus dictionary */
    if (!py_sensory_stimulus.contains("time")) {
      RCLCPP_ERROR(*logger_ptr, "sensory_stimulus does not contain the field: time.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    if (!py_sensory_stimulus.contains("state")) {
      RCLCPP_ERROR(*logger_ptr, "sensory_stimulus does not contain the field: state.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    if (!py_sensory_stimulus.contains("parameter")) {
      RCLCPP_ERROR(*logger_ptr, "sensory_stimulus does not contain the field: parameter.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    if (!py_sensory_stimulus.contains("duration")) {
      RCLCPP_ERROR(*logger_ptr, "sensory_stimulus does not contain the field: duration.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    if (!py::isinstance<py::float_>(py_sensory_stimulus["time"])) {
      RCLCPP_ERROR(*logger_ptr, "'time' field of 'sensory_stimulus' dictionary should be of type float.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    if (!py::isinstance<py::int_>(py_sensory_stimulus["state"])) {
      RCLCPP_ERROR(*logger_ptr, "'state' field of 'sensory_stimulus' dictionary should be of type int.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    if (!py::isinstance<py::int_>(py_sensory_stimulus["parameter"])) {
      RCLCPP_ERROR(*logger_ptr, "'parameter' field of 'sensory_stimulus' dictionary should be of type int.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    if (!py::isinstance<py::float_>(py_sensory_stimulus["duration"])) {
      RCLCPP_ERROR(*logger_ptr, "'duration' field of 'sensory_stimulus' dictionary should be of type float.");
      state = WrapperState::ERROR;
      success = false;

      return {success, trial, trigger_labjack, request_sensory_stimulus};
    }

    /* Convert each field from the dictionary to the ROS message. */
    output_sensory_stimulus.time = py_sensory_stimulus["time"].cast<double_t>();
    output_sensory_stimulus.state = py_sensory_stimulus["state"].cast<uint16_t>();
    output_sensory_stimulus.parameter = py_sensory_stimulus["parameter"].cast<uint16_t>();
    output_sensory_stimulus.duration = py_sensory_stimulus["duration"].cast<double_t>();

    request_sensory_stimulus = true;
  }

  return {success, trial, trigger_labjack, request_sensory_stimulus};
}

rclcpp::Logger* DeciderWrapper::logger_ptr = nullptr;
