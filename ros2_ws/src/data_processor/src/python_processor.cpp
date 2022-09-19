//
// Created by alqio on 1.9.2022.
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
  python_init_name = PyUnicode_FromString("init");
  python_close_name = PyUnicode_FromString("close");
}

std::vector<FpgaEvent> PythonProcessor::init() {
  PyObject_CallMethodObjArgs(python_instance, python_init_name, nullptr);
  std::vector<FpgaEvent> events;
  return events;
}

PyObject *PythonProcessor::make_list(std::vector<double> data) {
  PyObject *l = PyList_New(data.size());
  for (size_t i = 0; i < data.size(); i++) {
    PyList_SetItem(l, i, PyFloat_FromDouble(data[i]));
  }
  return l;
}

fpga_interfaces::msg::ChargeEvent PythonProcessor::parse_charge_event(PyObject *event) {
  auto charge_event = fpga_interfaces::msg::ChargeEvent();

  auto channel = PyObject_GetAttrString(event, "channel");
  if (channel == nullptr) {
    PyErr_Print();
    std::cout << "Error on event channel" << std::endl;
  }

  auto event_info_as_pyobject = PyObject_GetAttrString(event, "event_info");
  if (event_info_as_pyobject == nullptr) {
    PyErr_Print();
    std::cout << "Error on event_info_as_pyobject channel" << std::endl;
  }
  auto event_id = PyDict_GetItemString(event_info_as_pyobject, "event_id");
  if (event_id == nullptr) {
    PyErr_Print();
    std::cout << "Error on event_id channel" << std::endl;
  }
  auto execution_condition = PyDict_GetItemString(event_info_as_pyobject, "execution_condition");
  if (execution_condition == nullptr) {
    PyErr_Print();
    std::cout << "Error on execution_condition channel" << std::endl;
  }
  auto time_us = PyDict_GetItemString(event_info_as_pyobject, "time_us");
  if (time_us == nullptr) {
    PyErr_Print();
    std::cout << "Error on time_us channel" << std::endl;
  }
  auto target_voltage = PyObject_GetAttrString(event, "target_voltage");
  if (target_voltage == nullptr) {
    PyErr_Print();
    std::cout << "Error on target_voltage channel" << std::endl;
  }

  charge_event.channel = PyLong_AsUnsignedLong(channel);
  charge_event.target_voltage = PyLong_AsUnsignedLong(target_voltage);
  charge_event.event_info.event_id = PyLong_AsUnsignedLong(event_id);
  charge_event.event_info.execution_condition = PyLong_AsUnsignedLong(execution_condition);
  charge_event.event_info.time_us = PyLong_AsUnsignedLong(time_us);
  //Py_DECREF(channel);
  //Py_DECREF(target_voltage);
  //Py_DECREF(event_info_as_pyobject);
  //Py_DECREF(event_id);
  //Py_DECREF(time_us);

  return charge_event;
}

fpga_interfaces::msg::StimulationPulseEvent PythonProcessor::parse_stimulation_event(PyObject *event) {
  auto stimulation_event = fpga_interfaces::msg::StimulationPulseEvent();

  auto channel = PyObject_GetAttrString(event, "channel");
  if (channel == nullptr) {
    PyErr_Print();
    std::cout << "Error on event channel" << std::endl;
  }

  auto event_info_as_pyobject = PyObject_GetAttrString(event, "event_info");
  auto event_id = PyDict_GetItemString(event_info_as_pyobject, "event_id");
  auto execution_condition = PyDict_GetItemString(event_info_as_pyobject, "execution_condition");
  auto time_us = PyDict_GetItemString(event_info_as_pyobject, "time_us");

  auto pieces = PyObject_GetAttrString(event, "pieces");
  for (auto i = 0; i < PyList_Size(pieces); i++) {
    auto piece_as_pyobject = PyList_GetItem(pieces, i);
    auto mode = PyDict_GetItemString(piece_as_pyobject, "mode");
    auto duration_in_ticks = PyDict_GetItemString(piece_as_pyobject, "duration_in_ticks");

    auto piece = fpga_interfaces::msg::StimulationPulsePiece();
    piece.mode = PyLong_AsUnsignedLong(mode);
    piece.duration_in_ticks = PyLong_AsUnsignedLong(duration_in_ticks);

    stimulation_event.pieces.push_back(piece);
    //Py_DECREF(mode);
    //Py_DECREF(duration_in_ticks);
  }

  stimulation_event.channel = PyLong_AsUnsignedLong(channel);
  stimulation_event.event_info.event_id = PyLong_AsUnsignedLong(event_id);
  stimulation_event.event_info.execution_condition = PyLong_AsUnsignedLong(execution_condition);
  stimulation_event.event_info.time_us = PyLong_AsUnsignedLong(time_us);
  //Py_DECREF(channel);
  //Py_DECREF(event_info_as_pyobject);
  //Py_DECREF(event_id);
  //Py_DECREF(time_us);

  return stimulation_event;
}

std::vector<FpgaEvent> PythonProcessor::parse_pyobject_events(std::vector<PyObject *> events) {
  std::vector<FpgaEvent> fpga_events;

  for (auto event_as_pyobject: events) {
    FpgaEvent event;

    auto event_type_as_pyobject = PyObject_GetAttrString(event_as_pyobject, "event_type");
    auto event_type = PyUnicode_AsUTF8(event_type_as_pyobject);

    if (strcmp(event_type, "stimulation") == 0) {
      auto stimulation_event = parse_stimulation_event(event_as_pyobject);
      event.stimulation_pulse_event = stimulation_event;
      event.event_type = STIMULATION_PULSE_EVENT;
    } else if (strcmp(event_type, "charge") == 0) {
      auto charge_event = parse_charge_event(event_as_pyobject);
      event.charge_event = charge_event;
      event.event_type = CHARGE_EVENT;
    } else {
      std::wcout << "Unknown event type" << std::endl;
    }
    fpga_events.push_back(event);
  }

  return fpga_events;
}

std::vector<FpgaEvent> PythonProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  auto list = make_list(data.channel_datapoint);
  auto time = PyFloat_FromDouble(data.time);
  auto first_sample_of_experiment = PyBool_FromLong(data.first_sample_of_experiment ? 1L : 0L);
  //std::cout << "in data received" << std::endl;

  auto result = PyObject_CallMethodObjArgs(python_instance, python_data_received_name, list, time,
                                           first_sample_of_experiment, nullptr);

  if (!PyList_Check(result)) {
    std::cout << "error in call method" << std::endl;
    PyErr_Print();
  }

  std::vector<PyObject *> events;

  for (auto i = 0; i < PyList_Size(result); i++) {
    events.push_back(PyList_GetItem(result, i));
  }

  auto fpga_events = parse_pyobject_events(events);
  return fpga_events;
}

std::vector<FpgaEvent> PythonProcessor::close() {
  std::cout << "python close" << std::endl;
  PyObject_CallMethodObjArgs(python_instance, python_close_name, nullptr);
  Py_FinalizeEx();
  std::vector<FpgaEvent> events;
  return events;
}