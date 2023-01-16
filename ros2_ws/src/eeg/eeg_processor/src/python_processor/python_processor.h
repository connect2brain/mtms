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

  ~PythonProcessor();

  std::vector<Event> init();

  std::vector<mtms_interfaces::msg::EegDatapoint> raw_eeg_received(mtms_interfaces::msg::EegDatapoint sample);

  std::vector<Event> cleaned_eeg_received(mtms_interfaces::msg::EegDatapoint sample);

  std::vector<Event> present_stimulus_received(mtms_interfaces::msg::Event event);

private:
  static PyObject *convert_vector_to_pyobject(std::vector<double> data);
  static std::vector<double> convert_pyobject_to_vector(PyObject * data);

  static std::vector<Event> convert_pyobject_events_to_events(std::vector<PyObject *> events);

  static std::vector<mtms_interfaces::msg::EegDatapoint> convert_pyobject_samples_to_samples(std::vector<PyObject *> samples);


  static event_interfaces::msg::Event parse_event(PyObject *event);

  static event_interfaces::msg::Charge parse_charge(PyObject *event);
  static event_interfaces::msg::Discharge parse_discharge(PyObject *event);
  static event_interfaces::msg::SignalOut parse_signal_out(PyObject *event);
  static event_interfaces::msg::Pulse parse_pulse(PyObject *event);

  PyObject *python_init_name, *python_data_received_name, *python_close_name;
  PyObject *script_name, *python_module, *python_module_dict, *python_class, *python_instance;
};


#endif //EEG_PROCESSOR_PYTHON_PROCESSOR_H