//
// Created by alqio on 1.9.2022.
//

#ifndef DATA_PROCESSOR_PYTHON_PROCESSOR_H
#define DATA_PROCESSOR_PYTHON_PROCESSOR_H

#include "Python.h"
#include "processor.h"
#include "iostream"

class PythonProcessor : public ProcessorWrapper {
public:
  PythonProcessor(std::string script_path);
  void init();

  void data_received(int data);

  int close();

private:
  std::string python_init_name, python_data_received_name;
  PyObject *script_name, *python_module, *python_module_dict, *python_class, *python_instance, *python_args, *python_value;
};


#endif //DATA_PROCESSOR_PYTHON_PROCESSOR_H
