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
  python_close_name = PyUnicode_FromString("end_experiment");
}

std::vector<Event> PythonProcessor::init() {
  auto result = PyObject_CallMethodObjArgs(python_instance, python_init_name, nullptr);
  if (!PyList_Check(result)) {
    std::cout << "Error in call init method. Ensure you are returning a list" << std::endl;
    PyErr_Print();
  }
  std::vector<PyObject *> events;

  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto events_ = convert_pyobject_events_to_events(events);

  return events_;
}

PyObject *PythonProcessor::convert_vector_to_pyobject(std::vector<double> data) {
  PyObject *l = PyList_New(data.size());
  for (size_t i = 0; i < data.size(); i++) {
    PyList_SetItem(l, i, PyFloat_FromDouble(data[i]));
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

event_interfaces::msg::SignalOut PythonProcessor::parse_signal_out(PyObject *event) {
  auto signal_out = event_interfaces::msg::SignalOut();

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

  signal_out.port = PyLong_AsUnsignedLong(port);
  signal_out.duration_us = PyLong_AsUnsignedLong(duration_us);

  signal_out.event_info = parse_event_info(event);

  Py_DECREF(port);
  Py_DECREF(duration_us);

  return signal_out;
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
  std::vector<Event> events_;

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

    } else if (event_type == SIGNAL_OUT) {
      auto signal_out = parse_signal_out(event_as_pyobject);
      event.signal_out = signal_out;
      event.event_type = SIGNAL_OUT;

    } else {
      RCLCPP_WARN(rclcpp::get_logger("eeg_processor"), "Unknown event type: %lu", event_type);
    }

    events_.push_back(event);

    Py_DECREF(event_type_as_pyobject);
  }

  return events_;
}

std::vector<Event> PythonProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  auto list = convert_vector_to_pyobject(data.eeg_channels);
  auto time = PyFloat_FromDouble(data.time);
  auto first_sample_of_experiment = PyBool_FromLong(data.first_sample_of_experiment ? 1L : 0L);

  auto result = PyObject_CallMethodObjArgs(
      python_instance,
      python_data_received_name,
      list,
      time,
      first_sample_of_experiment,
      nullptr
  );

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

  auto events_ = convert_pyobject_events_to_events(events);

  Py_DECREF(result);

  return events_;
}

std::vector<Event> PythonProcessor::close() {
  auto result = PyObject_CallMethodObjArgs(python_instance, python_close_name, nullptr);

  if (!PyList_Check(result)) {
    std::cout << "Error in call close method. Ensure you are returning a list" << std::endl;
    PyErr_Print();
  }
  std::vector<PyObject *> events;

  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto events_ = convert_pyobject_events_to_events(events);

  Py_FinalizeEx();

  return events_;
}
