//
// Created by alqio on 1.9.2022.
//

#include "matlab_processor.h"


MatlabProcessor::MatlabProcessor(const std::string &script_path) {
  std::cout << "Starting matlab" << std::endl;

  matlab = matlab::engine::startMATLAB();
  matlab::engine::connectMATLAB();
  std::cout << "MATLAB started" << std::endl;

  auto path = "addpath " + script_path;
  std::cout << path << std::endl;
  matlab->eval(matlab::engine::convertUTF8StringToUTF16String(path));

  matlab_data.resize(50 * 62, -1);
}

std::vector<Event> MatlabProcessor::init() {
  std::vector<Event> events;
  return events;
}

void print_vector2d(std::vector<std::vector<double>> vec) {
  for (unsigned i = 0; i < vec.size(); i++) {
    for (unsigned j = 0; j < vec[i].size(); j++) {
      std::cout << vec[i][j] << " ";
    }
    std::cout << std::endl;
  }
}

void print_vector(std::vector<double> vec, unsigned rows, unsigned cols) {
  for (unsigned i = 0; i < rows; i++) {
    for (unsigned j = 0; j < cols; j++) {
      std::cout << vec[i * cols + j] << " ";
    }
    std::cout << std::endl;
  }
}

std::vector<Event> MatlabProcessor::present_stimulus_received(mtms_interfaces::msg::Event event) {}


std::vector<mtms_interfaces::msg::EegDatapoint> MatlabProcessor::raw_eeg_received(mtms_interfaces::msg::EegDatapoint sample) {
}


std::vector<Event> MatlabProcessor::cleaned_eeg_received(mtms_interfaces::msg::EegDatapoint sample) {
  std::vector<Event> fpga_events;

  matlab::data::TypedArray<double> matlab_data_array = factory.createArray(
      {50, 62},
      matlab_data.begin(),
      matlab_data.end(),
      matlab::data::InputLayout::ROW_MAJOR
  );
  auto matlab_data_array_dims = matlab_data_array.getDimensions();

  auto dim = matlab::data::ArrayDimensions(sample.eeg_channels.size());
  auto matlab_new_sample = factory.createArray<double>(
      {1, sample.eeg_channels.size()},
      sample.eeg_channels.data(),
      sample.eeg_channels.data() + sample.eeg_channels.size()
  );


  std::vector<matlab::data::Array> args({
                                            matlab_data_array,
                                            matlab_new_sample
                                        });

  std::vector<matlab::data::Array> results = matlab->feval(u"MatlabProcessorScript", 2, args);

  auto new_data = results[0];
  auto new_data_typed = (matlab::data::TypedArray<double>) new_data;

  matlab::data::ArrayDimensions new_data_dims = new_data_typed.getDimensions();

  for (unsigned i = 0; i < new_data_dims[0]; i++) {
    for (unsigned j = 0; j < new_data_dims[1]; j++) {
      matlab_data[i * new_data_dims[1] + j] = new_data_typed[i][j];
    }
  }

  auto events = results[1];
  auto events_struct_array = (matlab::data::StructArray) events;

  matlab::data::ArrayDimensions dims = events_struct_array.getDimensions();

  auto fields = events_struct_array.getFieldNames();
  std::vector<matlab::data::MATLABFieldIdentifier> field_names(fields.begin(), fields.end());

  for (unsigned event_index = 0; event_index < events_struct_array.getNumberOfElements(); event_index++) {
    matlab_event event;

    for (auto field: field_names) {
      auto field_name = field.operator std::string();

      if (field_name == "channel") {
        matlab::data::TypedArrayRef<uint8_t> field_value = events_struct_array[event_index][field];
        event.channel = field_value[0];

      } else if (field_name == "event_type") {
        matlab::data::TypedArrayRef<uint8_t> field_value = events_struct_array[event_index][field];
        event.event_type = field_value[0];

      } else if (field_name == "target_voltage") {
        matlab::data::TypedArrayRef<uint16_t> field_value = events_struct_array[event_index][field];
        event.target_voltage = field_value[0];

      } else if (field_name == "event") {
        matlab::data::StructArray event_info = events_struct_array[event_index][field];
        auto sub_struct_fields = event_info.getFieldNames();
        std::vector<matlab::data::MATLABFieldIdentifier> sub_struct_field_names(sub_struct_fields.begin(),
                                                                                sub_struct_fields.end());

        for (auto sub_struct_field: sub_struct_field_names) {
          auto sub_struct_field_name = sub_struct_field.operator std::string();
          if (sub_struct_field_name == "id") {
            matlab::data::TypedArrayRef<uint16_t> field_value = event_info[0][sub_struct_field];
            event.b_event.id = field_value[0];

          } else if (sub_struct_field_name == "execution_condition") {
            matlab::data::TypedArrayRef<uint8_t> field_value = event_info[0][sub_struct_field];
            event.b_event.execution_condition = field_value[0];

          } else if (sub_struct_field_name == "time") {
            matlab::data::TypedArrayRef<double_t> field_value = event_info[0][sub_struct_field];
            event.b_event.time = field_value[0];

          } else {
            std::cout << "unknown event type" << std::endl;
          }
        }
      } else if (field_name == "waveform") {
        matlab::data::StructArray waveform = events_struct_array[event_index][field];
        auto sub_struct_fields = waveform.getFieldNames();
        std::vector<matlab::data::MATLABFieldIdentifier> sub_struct_field_names(sub_struct_fields.begin(),
                                                                                sub_struct_fields.end());

        matlab::data::ArrayDimensions waveform_dims = waveform.getDimensions();

        for (auto sub_struct_field: sub_struct_field_names) {
          auto sub_struct_field_name = sub_struct_field.operator std::string();
          for (unsigned i = 0; i < waveform_dims[1]; i++) {
            if (sub_struct_field_name == "waveform_phase") {
              matlab::data::TypedArrayRef<uint8_t> field_value = waveform[i][sub_struct_field];
              event.waveform[i].waveform_phase = field_value[0];

            } else if (sub_struct_field_name == "duration_in_ticks") {
              matlab::data::TypedArrayRef<uint16_t> field_value = waveform[i][sub_struct_field];
              event.waveform[i].duration_in_ticks = field_value[0];

            } else {
              std::cout << "unknown event type" << std::endl;
            }
          }
        }
      } else {
        std::cout << "unknown event type" << std::endl;
      }

    }
    //print_matlab_fpga_event(event);
    auto fpga_event = convert_matlab_event_to_event(event);
    fpga_events.push_back(fpga_event);
  }
  //print_vector(matlab_data, 50, 62);
  return fpga_events;
}

MatlabProcessor::~MatlabProcessor() {
  matlab::engine::terminateEngineClient();
  std::cout << "Closed matlab engine client" << std::endl;
}