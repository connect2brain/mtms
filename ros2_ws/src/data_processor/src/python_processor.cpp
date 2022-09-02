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
}

void PythonProcessor::init() {
  PyObject_CallMethodObjArgs(python_instance, python_init_name, nullptr);
}

PyObject *PythonProcessor::make_list(std::vector<double> data) {
  PyObject *l = PyList_New(data.size());
  for (size_t i = 0; i < data.size(); i++) {
    PyList_SetItem(l, i, PyFloat_FromDouble(data[i]));
  }
  return l;
}

std::vector<fpga_interfaces::msg::StimulationPulseEvent>
PythonProcessor::parse_pyobject_events(std::vector<PyObject *> events) {
  std::vector<fpga_interfaces::msg::StimulationPulseEvent> stimulation_events;

  for (auto event: events) {
    auto stimulation_event = fpga_interfaces::msg::StimulationPulseEvent();

    auto channel_as_pyobject = PyObject_GetAttrString(event, "channel");
    if (!PyList_Check(channel_as_pyobject)) {
      std::cout << "error" << std::endl;
      PyErr_Print();
    }
    auto channel = (uint8_t) PyLong_AsLong(channel_as_pyobject);

    auto event_info_as_pyobject = PyObject_GetAttrString(event, "event_info");
    auto event_id = PyObject_GetAttrString(event_info_as_pyobject, "event_id");
    auto execution_condition = PyObject_GetAttrString(event_info_as_pyobject, "execution_condition");
    auto time_us = PyObject_GetAttrString(event_info_as_pyobject, "time_us");

    auto pieces = PyObject_GetAttrString(event, "pieces");
    for (auto i = 0; i < PyList_Size(pieces); i++) {
      auto piece_as_pyobject = PyList_GetItem(pieces, i);
      auto mode = PyObject_GetAttrString(piece_as_pyobject, "mode");
      auto duration_in_ticks = PyObject_GetAttrString(piece_as_pyobject, "duration_in_ticks");

      auto piece = fpga_interfaces::msg::StimulationPulsePiece();
      piece.mode = PyLong_AsSize_t(mode);
      piece.duration_in_ticks = PyLong_AsSize_t(duration_in_ticks);

      stimulation_event.pieces.push_back(piece);
    }

    stimulation_event.channel = channel;
    stimulation_event.event_info.event_id = PyLong_AsSize_t(event_id);
    stimulation_event.event_info.execution_condition = PyLong_AsSize_t(execution_condition);
    stimulation_event.event_info.time_us = PyLong_AsSize_t(time_us);
    stimulation_events.push_back(stimulation_event);
  }

  return stimulation_events;
}

std::vector<fpga_interfaces::msg::StimulationPulseEvent>
PythonProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  auto list = make_list(data.channel_datapoint);
  auto time = PyFloat_FromDouble(data.time);
  auto first_sample_of_experiment = PyBool_FromLong(data.first_sample_of_experiment ? 1L : 0L);

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
  std::cout << "event count: " << events.size() << std::endl;

  auto stimulation_events = parse_pyobject_events(events);
  std::cout << "stimulation_events count: " << stimulation_events.size() << std::endl;
  return stimulation_events;
}

int PythonProcessor::close() {
  std::cout << "finalized python" << std::endl;
  return Py_FinalizeEx();
}