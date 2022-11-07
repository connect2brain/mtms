//
// Created by alqio on 1.9.2022.
//

#ifndef EEG_PROCESSOR_PYTHON_PROCESSOR_H
#define EEG_PROCESSOR_PYTHON_PROCESSOR_H

#include "Python.h"
#include "processor.h"
#include "iostream"

class PythonProcessor : public ProcessorWrapper {
public:
  PythonProcessor(std::string script_path);

  std::vector<FpgaEvent> init();

  std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data);

  std::vector<FpgaEvent> close();

private:
  static PyObject *convert_vector_to_pyobject(std::vector<double> data);

  static std::vector<FpgaEvent> convert_pyobject_events_to_fpga_events(std::vector<PyObject *> events);

  static fpga_interfaces::msg::Charge parse_charge(PyObject *event);
  static fpga_interfaces::msg::Discharge parse_discharge(PyObject *event);
  static fpga_interfaces::msg::Event parse_event(PyObject *event);

  static fpga_interfaces::msg::Pulse parse_pulse(PyObject *event);

  PyObject *python_init_name, *python_data_received_name, *python_close_name;
  PyObject *script_name, *python_module, *python_module_dict, *python_class, *python_instance;
};


#endif //EEG_PROCESSOR_PYTHON_PROCESSOR_H
