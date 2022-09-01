//
// Created by alqio on 1.9.2022.
//

#include "headers/python_processor.h"

PythonProcessor::PythonProcessor(std::string script_path) {
  setenv("PYTHONPATH",".",1);
  Py_Initialize();
  PyRun_SimpleString("print('moi')");
  PyRun_SimpleString("from processors.python_processor import Processor;p = Processor(); p.init()");

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

  python_data_received_name = "data_received";
  python_init_name = "init";
}

void PythonProcessor::init() {
  PyObject_CallMethod(python_instance, python_init_name.c_str(), nullptr);
}

void PythonProcessor::data_received(int data) {
  PyObject_CallMethod(python_instance, python_data_received_name.c_str(), "(i)", data);
}

int PythonProcessor::close() {
  return Py_FinalizeEx();
}