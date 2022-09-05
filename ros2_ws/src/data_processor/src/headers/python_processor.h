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

  std::vector<FpgaEvent> data_received(mtms_interfaces::msg::EegDatapoint data);

  int close();

private:
  PyObject *make_list(std::vector<double> data);

  std::vector<FpgaEvent> parse_pyobject_events(std::vector<PyObject *> events);

  fpga_interfaces::msg::ChargeEvent parse_charge_event(PyObject *event);

  fpga_interfaces::msg::StimulationPulseEvent parse_stimulation_event(PyObject *event);

  PyObject *python_init_name, *python_data_received_name;
  PyObject *script_name, *python_module, *python_module_dict, *python_class, *python_instance, *python_args, *python_value;
};


#endif //DATA_PROCESSOR_PYTHON_PROCESSOR_H
