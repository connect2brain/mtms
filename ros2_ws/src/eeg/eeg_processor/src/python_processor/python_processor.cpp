//
// Created by alqio on 1.9.2022.
// Recommended reading about reference counting: https://docs.python.org/3/c-api/intro.html#reference-count-details
// Note the difference between "New reference" and "Borrowed reference" and what kind of reference each function returns
//

#include "python_processor.h"
#include "rclcpp/rclcpp.hpp"


PythonProcessor::PythonProcessor(std::string script_path) {
  setenv("PYTHONPATH", ".", 1);
  Py_Initialize();

  script_name = PyUnicode_FromString(script_path.c_str());
  python_module = PyImport_Import(script_name);

  if (python_module == nullptr) {
    PyErr_Print();
    std::cerr << "Failed to import module: " << script_name << std::endl;
    return;
  }
  Py_DECREF(script_name);

  python_module_dict = PyModule_GetDict(python_module);
  if (python_module_dict == nullptr) {
    PyErr_Print();
    std::cerr << "Fails to get module dictionary from: " << python_module << std::endl;
    return;
  }
  Py_DECREF(python_module);

  python_class = PyDict_GetItemString(python_module_dict, "Processor");

  if (python_class == nullptr) {
    PyErr_Print();
    std::cerr << "Failed to get class" << std::endl;
    return;
  }
  Py_DECREF(python_module_dict);

  if (PyCallable_Check(python_class) == 1) {
    python_instance = PyObject_CallObject(python_class, nullptr);
    if (python_instance == NULL) {
      PyErr_Print();
    }

    Py_DECREF(python_class);
  } else {
    std::cout << "Cannot instantiate python class" << std::endl;
    Py_DECREF(python_class);
    return;
  }
  python_data_received_name = PyUnicode_FromString("data_received");
  python_init_name = PyUnicode_FromString("init_experiment");
  python_end_name = PyUnicode_FromString("end_experiment");
}

std::vector<Event> PythonProcessor::init() {
  PyObject* result = PyObject_CallMethodObjArgs(python_instance, python_init_name, nullptr);

  /* Check Python script for crashing. */
  if (result == nullptr) {
    PyErr_Print();

    /* Shut down ROS node and return an empty vector to fail gracefully. */
    rclcpp::shutdown();
    return std::vector<Event>();
  }

  if (!PyList_Check(result)) {
    std::cout << "Error in call init method. Ensure you are returning a list" << std::endl;
    PyErr_Print();
  }

  std::vector<PyObject *> events;
  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto events_out = convert_pyobject_events_to_events(events);
  return events_out;
}

std::vector<Event> PythonProcessor::end_experiment() {
  PyObject* result = PyObject_CallMethodObjArgs(python_instance, python_end_name, nullptr);

  /* Check Python script for crashing. */
  if (result == nullptr) {
    PyErr_Print();

    /* Shut down ROS node and return an empty vector to fail gracefully. */
    rclcpp::shutdown();
    return std::vector<Event>();
  }

  if (!PyList_Check(result)) {
    std::cout << "Error in call end_experiment method. Ensure you are returning a list" << std::endl;
    PyErr_Print();
  }

  std::vector<PyObject *> events;
  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto events_out = convert_pyobject_events_to_events(events);
  return events_out;
}


PyObject *PythonProcessor::convert_vector_to_pyobject(std::vector<double> data) {
  PyObject *l = PyList_New(data.size());
  for (size_t i = 0; i < data.size(); i++) {
    PyList_SetItem(l, i, PyFloat_FromDouble(data[i]));
  }
  return l;
}

std::vector<double> PythonProcessor::convert_pyobject_to_vector(PyObject *data) {
  std::vector<double> l;

  Py_ssize_t size = PyList_Size(data);

  PyObject *item;
  for (size_t i = 0; i < size; i++) {
    item = PyList_GetItem(data, i);
    if (!PyFloat_Check(item)) {
      RCLCPP_WARN(rclcpp::get_logger("eeg_preprocessor"), "Index %d of returned sample was not a float.", i);
      continue;
    }
    l.push_back(PyFloat_AsDouble(item));
  }
  return l;
}


event_interfaces::msg::EventInfo PythonProcessor::parse_event_info(PyObject *event) {
  auto event_info_as_pyobject = PyObject_GetAttrString(event, "event_info");
  if (event_info_as_pyobject == nullptr) {
    PyErr_Print();
    std::cout << "Error on event_info_as_pyobject" << std::endl;
  }
  auto id = PyDict_GetItemString(event_info_as_pyobject, "id");
  if (id == nullptr) {
    PyErr_Print();
    std::cout << "Error on id" << std::endl;
  }
  auto execution_condition = PyDict_GetItemString(event_info_as_pyobject, "execution_condition");
  if (execution_condition == nullptr) {
    PyErr_Print();
    std::cout << "Error on execution_condition" << std::endl;
  }
  auto execution_time = PyDict_GetItemString(event_info_as_pyobject, "execution_time");
  if (execution_time == nullptr) {
    PyErr_Print();
    std::cout << "Error on execution_time" << std::endl;
  }

  event_interfaces::msg::EventInfo event_info;

  event_info.execution_time = PyFloat_AsDouble(execution_time);
  event_info.execution_condition.value = PyLong_AsUnsignedLong(execution_condition);
  event_info.id = PyLong_AsUnsignedLong(id);

  Py_DECREF(event_info_as_pyobject);

  return event_info;
}

event_interfaces::msg::Charge PythonProcessor::parse_charge(PyObject *event) {
  auto charge = event_interfaces::msg::Charge();

  auto channel = PyObject_GetAttrString(event, "channel");
  if (channel == nullptr) {
    PyErr_Print();
    std::cout << "Error on event channel" << std::endl;
  }

  auto target_voltage = PyObject_GetAttrString(event, "target_voltage");
  if (target_voltage == nullptr) {
    PyErr_Print();
    std::cout << "Error on target_voltage channel" << std::endl;
  }

  charge.channel = PyLong_AsUnsignedLong(channel);
  charge.target_voltage = PyLong_AsUnsignedLong(target_voltage);

  charge.event_info = parse_event_info(event);

  Py_DECREF(channel);
  Py_DECREF(target_voltage);

  return charge;
}

event_interfaces::msg::Discharge PythonProcessor::parse_discharge(PyObject *event) {
  auto discharge = event_interfaces::msg::Discharge();

  auto channel = PyObject_GetAttrString(event, "channel");
  if (channel == nullptr) {
    PyErr_Print();
    std::cout << "Error on event channel" << std::endl;
  }

  auto target_voltage = PyObject_GetAttrString(event, "target_voltage");
  if (target_voltage == nullptr) {
    PyErr_Print();
    std::cout << "Error on target_voltage" << std::endl;
  }

  discharge.channel = PyLong_AsUnsignedLong(channel);
  discharge.target_voltage = PyLong_AsUnsignedLong(target_voltage);

  discharge.event_info = parse_event_info(event);

  Py_DECREF(channel);
  Py_DECREF(target_voltage);

  return discharge;
}

event_interfaces::msg::TriggerOut PythonProcessor::parse_trigger_out(PyObject *event) {
  auto trigger_out = event_interfaces::msg::TriggerOut();

  auto port = PyObject_GetAttrString(event, "port");

  if (port == nullptr) {
    PyErr_Print();
    std::cout << "Error on event port" << std::endl;
  }

  auto duration_us = PyObject_GetAttrString(event, "duration_us");
  if (duration_us == nullptr) {
    PyErr_Print();
    std::cout << "Error on duration_us" << std::endl;
  }

  trigger_out.port = PyLong_AsUnsignedLong(port);
  trigger_out.duration_us = PyLong_AsUnsignedLong(duration_us);

  trigger_out.event_info = parse_event_info(event);

  Py_DECREF(port);
  Py_DECREF(duration_us);

  return trigger_out;
}

event_interfaces::msg::Stimulus PythonProcessor::parse_stimulus(PyObject *event) {
  auto stimulus = event_interfaces::msg::Stimulus();
  auto state = PyObject_GetAttrString(event, "state");
  if (state == nullptr) {
    PyErr_Print();
    std::cout << "Error on event state" << std::endl;
  }
  stimulus.state = PyLong_AsUnsignedLong(state);
  stimulus.event_info = parse_event_info(event);

  Py_DECREF(state);

  return stimulus;
}

event_interfaces::msg::Pulse PythonProcessor::parse_pulse(PyObject *event) {
  auto pulse = event_interfaces::msg::Pulse();

  auto channel = PyObject_GetAttrString(event, "channel");
  if (channel == nullptr) {
    PyErr_Print();
    std::cout << "Error on event channel" << std::endl;
  }

  auto waveform = PyObject_GetAttrString(event, "waveform");
  if (waveform == nullptr) {
    PyErr_Print();
    std::cout << "Error on event waveform" << std::endl;
  }

  for (auto i = 0; i < PyList_Size(waveform); i++) {
    auto piece_as_pyobject = PyList_GetItem(waveform, i);
    auto waveform_phase = PyDict_GetItemString(piece_as_pyobject, "waveform_phase");
    auto duration_in_ticks = PyDict_GetItemString(piece_as_pyobject, "duration_in_ticks");

    auto piece = event_interfaces::msg::WaveformPiece();
    piece.waveform_phase.value = PyLong_AsUnsignedLong(waveform_phase);
    piece.duration_in_ticks = PyLong_AsUnsignedLong(duration_in_ticks);

    pulse.waveform.push_back(piece);
  }

  pulse.channel = PyLong_AsUnsignedLong(channel);
  pulse.event_info = parse_event_info(event);

  Py_DECREF(channel);
  Py_DECREF(waveform);

  return pulse;
}

std::vector<Event> PythonProcessor::convert_pyobject_events_to_events(std::vector<PyObject *> events) {
  std::vector<Event> events_out;

  for (auto event_as_pyobject: events) {
    Event event;

    auto event_type_as_pyobject = PyObject_GetAttrString(event_as_pyobject, "event_type");
    auto event_type = PyLong_AsUnsignedLong(event_type_as_pyobject);

    if (event_type == PULSE) {
      auto pulse = parse_pulse(event_as_pyobject);
      event.pulse = pulse;
      event.event_type = PULSE;

    } else if (event_type == CHARGE) {
      auto charge = parse_charge(event_as_pyobject);
      event.charge = charge;
      event.event_type = CHARGE;

    } else if (event_type == DISCHARGE) {
      auto discharge = parse_discharge(event_as_pyobject);
      event.discharge = discharge;
      event.event_type = DISCHARGE;

    } else if (event_type == TRIGGER_OUT) {
      auto trigger_out = parse_trigger_out(event_as_pyobject);
      event.trigger_out = trigger_out;
      event.event_type = TRIGGER_OUT;

    } else if (event_type == STIMULUS) {
      auto stimulus = parse_stimulus(event_as_pyobject);
      event.stimulus = stimulus;
      event.event_type = STIMULUS;

    } else {
      RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Unknown event type: %lu", event_type);
    }

    events_out.push_back(event);

    Py_DECREF(event_type_as_pyobject);
  }

  return events_out;
}

std::vector<eeg_interfaces::msg::EegDatapoint>
PythonProcessor::convert_pyobject_samples_to_samples(std::vector<PyObject *> samples) {
  std::vector<eeg_interfaces::msg::EegDatapoint> new_samples;

  for (auto sample_as_pyobject: samples) {
    eeg_interfaces::msg::EegDatapoint sample;

    auto channel_data_as_pyobject = PyObject_GetAttrString(sample_as_pyobject, "sample");

    if (!PyList_Check(channel_data_as_pyobject)) {
      RCLCPP_ERROR(rclcpp::get_logger("eeg_preprocessor"),
                   "Error in call raw_eeg_received method. Ensure you are returning a list from python pre processor");
      PyErr_Print();
    }
    sample.eeg_channels = convert_pyobject_to_vector(channel_data_as_pyobject);

    auto time_as_pyobject = PyObject_GetAttrString(sample_as_pyobject, "time");
    sample.time = PyFloat_AsDouble(time_as_pyobject);

    auto first_as_pyobject = PyObject_GetAttrString(sample_as_pyobject, "first_sample_of_experiment");
    auto v = PyLong_AsLong(first_as_pyobject);
    sample.first_sample_of_experiment = v == 1;

    new_samples.push_back(sample);

  }

  return new_samples;

}

std::vector<eeg_interfaces::msg::EegDatapoint>
PythonProcessor::raw_eeg_received(eeg_interfaces::msg::EegDatapoint sample) {
  auto list = convert_vector_to_pyobject(sample.eeg_channels);
  auto time = PyFloat_FromDouble(sample.time);
  auto first_sample_of_experiment = PyBool_FromLong(sample.first_sample_of_experiment ? 1L : 0L);

  PyObject* result = PyObject_CallMethodObjArgs(
      python_instance,
      python_data_received_name,
      list,
      time,
      first_sample_of_experiment,
      nullptr
  );

  /* Check Python script for crashing. */
  if (result == nullptr) {
    PyErr_Print();

    /* Shut down ROS node and return an empty vector to fail gracefully. */
    rclcpp::shutdown();
    return std::vector<eeg_interfaces::msg::EegDatapoint>();
  }

  if (!PyList_Check(result)) {
    std::cout << "Error in call raw_eeg_received method. Ensure you are returning a list" << std::endl;
    PyErr_Print();
  }

  Py_DECREF(list);
  Py_DECREF(time);
  Py_DECREF(first_sample_of_experiment);

  std::vector<PyObject *> samples;

  for (auto i = 0; i < PyList_Size(result); i++) {
    samples.push_back(PyList_GetItem(result, i));
  }

  auto cleaned_samples = convert_pyobject_samples_to_samples(samples);

  Py_DECREF(result);

  return cleaned_samples;

}

std::vector<Event> PythonProcessor::present_stimulus_received(event_interfaces::msg::Stimulus event) {
  auto time = PyFloat_FromDouble(event.event_info.execution_time);
  auto state = PyLong_FromSize_t(event.state);

  PyObject* result = PyObject_CallMethodObjArgs(
      python_instance,
      python_data_received_name,
      time,
      state,
      nullptr
  );

  /* Check Python script for crashing. */
  if (result == nullptr) {
    PyErr_Print();

    /* Shut down ROS node and return an empty vector to fail gracefully. */
    rclcpp::shutdown();
    return std::vector<Event>();
  }

  if (!PyList_Check(result)) {
    std::cout << "Error in call present_stimulus_received method. Ensure you are returning a list" << std::endl;
    PyErr_Print();
  }

  Py_DECREF(time);

  std::vector<PyObject *> events;

  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto events_out = convert_pyobject_events_to_events(events);

  Py_DECREF(result);

  return events_out;
}


std::vector<Event> PythonProcessor::cleaned_eeg_received(eeg_interfaces::msg::EegDatapoint sample) {
  auto list = convert_vector_to_pyobject(sample.eeg_channels);
  auto time = PyFloat_FromDouble(sample.time);
  auto first_sample_of_experiment = PyBool_FromLong(sample.first_sample_of_experiment ? 1L : 0L);

  PyObject* result = PyObject_CallMethodObjArgs(
    python_instance,
    python_data_received_name,
    list,
    time,
    first_sample_of_experiment,
    nullptr
  );

  /* Check Python script for crashing. */
  if (result == nullptr) {
    PyErr_Print();

    /* Shut down ROS node and return an empty vector to fail gracefully. */
    rclcpp::shutdown();
    return std::vector<Event>();
  }

  if (!PyList_Check(result)) {
    std::cout << "Error in call data_received method. Ensure you are returning a list" << std::endl;
    PyErr_Print();
  }

  Py_DECREF(list);
  Py_DECREF(time);
  Py_DECREF(first_sample_of_experiment);

  std::vector<PyObject *> events;

  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto events_out = convert_pyobject_events_to_events(events);

  Py_DECREF(result);

  return events_out;
}

PythonProcessor::~PythonProcessor() {
  Py_FinalizeEx();
}
