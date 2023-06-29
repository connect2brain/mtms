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
  std::vector<eeg_interfaces::msg::EegDatapoint> raw_eeg_received(eeg_interfaces::msg::EegDatapoint sample);
  std::vector<Event> cleaned_eeg_received(eeg_interfaces::msg::EegDatapoint sample);
  std::vector<Event> present_stimulus_received(event_interfaces::msg::Stimulus event);
  std::vector<Event> end_experiment();

private:
  static PyObject *convert_vector_to_pyobject(std::vector<double> data);
  static std::vector<double> convert_pyobject_to_vector(PyObject * data);

  static std::vector<Event> convert_pyobject_events_to_events(std::vector<PyObject *> events);

  static event_interfaces::msg::EventInfo parse_event_info(PyObject *event);
  static std::vector<eeg_interfaces::msg::EegDatapoint> convert_pyobject_samples_to_samples(std::vector<PyObject *> samples);

  static event_interfaces::msg::Charge parse_charge(PyObject *event);
  static event_interfaces::msg::Discharge parse_discharge(PyObject *event);
  static event_interfaces::msg::TriggerOut parse_trigger_out(PyObject *event);
  static event_interfaces::msg::Pulse parse_pulse(PyObject *event);
  static event_interfaces::msg::Stimulus parse_stimulus(PyObject *event);

  PyObject *script_name;
  PyObject *python_module;
  PyObject *python_module_dict;
  PyObject *python_class;
  PyObject *python_instance;

  PyObject *function_name_data_received;
  PyObject *function_name_init_experiment;
  PyObject *function_name_end_experiment;
};


#endif //EEG_PROCESSOR_PYTHON_PROCESSOR_H
