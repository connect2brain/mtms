//
// Created by alqio on 1.9.2022.
// Recommended reading about reference counting: https://docs.python.org/3/c-api/intro.html#reference-count-details
// Note the difference between "New reference" and "Borrowed reference" and what kind of reference each function returns
//

#include "headers/python_processor.h"

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

std::vector<FpgaEvent> PythonProcessor::init() {
  PyObject_CallMethodObjArgs(python_instance, python_init_name, nullptr);
  std::vector<FpgaEvent> events;
  return events;
}

PyObject *PythonProcessor::convert_vector_to_pyobject(std::vector<double> data) {
  PyObject *l = PyList_New(data.size());
  for (size_t i = 0; i < data.size(); i++) {
    PyList_SetItem(l, i, PyFloat_FromDouble(data[i]));
  }
  return l;
}

fpga_interfaces::msg::EventInfo PythonProcessor::parse_event_info(PyObject *event) {
  auto event_info_as_pyobject = PyObject_GetAttrString(event, "event_info");
  if (event_info_as_pyobject == nullptr) {
    PyErr_Print();
    std::cout << "Error on event_info_as_pyobject" << std::endl;
  }
  auto event_id = PyDict_GetItemString(event_info_as_pyobject, "event_id");
  if (event_id == nullptr) {
    PyErr_Print();
    std::cout << "Error on event_id" << std::endl;
  }
  auto execution_condition = PyDict_GetItemString(event_info_as_pyobject, "execution_condition");
  if (execution_condition == nullptr) {
    PyErr_Print();
    std::cout << "Error on execution_condition" << std::endl;
  }
  auto time_us = PyDict_GetItemString(event_info_as_pyobject, "time_us");
  if (time_us == nullptr) {
    PyErr_Print();
    std::cout << "Error on time_us" << std::endl;
  }
  fpga_interfaces::msg::EventInfo event_info;
  event_info.time_us = PyLong_AsUnsignedLong(time_us);
  event_info.execution_condition = PyLong_AsUnsignedLong(execution_condition);
  event_info.event_id = PyLong_AsUnsignedLong(event_id);

  Py_DECREF(event_info_as_pyobject);

  return event_info;
}

fpga_interfaces::msg::ChargeEvent PythonProcessor::parse_charge_event(PyObject *event) {
  auto charge_event = fpga_interfaces::msg::ChargeEvent();

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

  charge_event.channel = PyLong_AsUnsignedLong(channel);
  charge_event.target_voltage = PyLong_AsUnsignedLong(target_voltage);

  charge_event.event_info = parse_event_info(event);

  Py_DECREF(channel);
  Py_DECREF(target_voltage);

  return charge_event;
}

fpga_interfaces::msg::DischargeEvent PythonProcessor::parse_discharge_event(PyObject *event) {
  auto discharge_event = fpga_interfaces::msg::DischargeEvent();

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

  discharge_event.channel = PyLong_AsUnsignedLong(channel);
  discharge_event.target_voltage = PyLong_AsUnsignedLong(target_voltage);

  discharge_event.event_info = parse_event_info(event);

  Py_DECREF(channel);
  Py_DECREF(target_voltage);

  return discharge_event;
}

fpga_interfaces::msg::StimulationPulseEvent PythonProcessor::parse_stimulation_event(PyObject *event) {
  auto stimulation_event = fpga_interfaces::msg::StimulationPulseEvent();

  auto channel = PyObject_GetAttrString(event, "channel");
  if (channel == nullptr) {
    PyErr_Print();
    std::cout << "Error on event channel" << std::endl;
  }

  auto pieces = PyObject_GetAttrString(event, "pieces");
  if (pieces == nullptr) {
    PyErr_Print();
    std::cout << "Error on event pieces" << std::endl;
  }

  for (auto i = 0; i < PyList_Size(pieces); i++) {
    auto piece_as_pyobject = PyList_GetItem(pieces, i);
    auto mode = PyDict_GetItemString(piece_as_pyobject, "mode");
    auto duration_in_ticks = PyDict_GetItemString(piece_as_pyobject, "duration_in_ticks");

    auto piece = fpga_interfaces::msg::StimulationPulsePiece();
    piece.mode = PyLong_AsUnsignedLong(mode);
    piece.duration_in_ticks = PyLong_AsUnsignedLong(duration_in_ticks);

    stimulation_event.pieces.push_back(piece);
  }

  stimulation_event.channel = PyLong_AsUnsignedLong(channel);
  stimulation_event.event_info = parse_event_info(event);
  Py_DECREF(channel);
  Py_DECREF(pieces);

  return stimulation_event;
}

std::vector<FpgaEvent> PythonProcessor::parse_pyobject_events(std::vector<PyObject *> events) {
  std::vector<FpgaEvent> fpga_events;

  for (auto event_as_pyobject: events) {
    FpgaEvent event;

    auto event_type_as_pyobject = PyObject_GetAttrString(event_as_pyobject, "event_type");
    auto event_type = PyLong_AsUnsignedLong(event_type_as_pyobject);

    if (event_type == STIMULATION_PULSE_EVENT) {
      auto stimulation_event = parse_stimulation_event(event_as_pyobject);
      event.stimulation_pulse_event = stimulation_event;
      event.event_type = STIMULATION_PULSE_EVENT;

    } else if (event_type == CHARGE_EVENT) {
      auto charge_event = parse_charge_event(event_as_pyobject);
      event.charge_event = charge_event;
      event.event_type = CHARGE_EVENT;

    } else if (event_type == DISCHARGE_EVENT) {
      auto discharge_event = parse_discharge_event(event_as_pyobject);
      event.discharge_event = discharge_event;
      event.event_type = DISCHARGE_EVENT;

    } else {
      std::wcout << "Unknown event type" << std::endl;
    }

    fpga_events.push_back(event);

    Py_DECREF(event_type_as_pyobject);
  }

  return fpga_events;
}

std::vector<FpgaEvent> PythonProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  auto list = convert_vector_to_pyobject(data.channel_datapoint);
  auto time = PyFloat_FromDouble(data.time);
  auto first_sample_of_experiment = PyBool_FromLong(data.first_sample_of_experiment ? 1L : 0L);

  auto result = PyObject_CallMethodObjArgs(python_instance, python_data_received_name, list, time,
                                           first_sample_of_experiment, nullptr);

  if (!PyList_Check(result)) {
    std::cout << "error in call method" << std::endl;
    PyErr_Print();
  }

  Py_DECREF(list);
  Py_DECREF(time);
  Py_DECREF(first_sample_of_experiment);

  std::vector<PyObject *> events;

  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto fpga_events = parse_pyobject_events(events);

  Py_DECREF(result);

  return fpga_events;
}

std::vector<FpgaEvent> PythonProcessor::close() {
  auto result = PyObject_CallMethodObjArgs(python_instance, python_close_name, nullptr);

  if (!PyList_Check(result)) {
    std::cout << "error in call method" << std::endl;
    PyErr_Print();
  }
  std::vector<PyObject *> events;

  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto fpga_events = parse_pyobject_events(events);

  Py_FinalizeEx();

  return fpga_events;
}
