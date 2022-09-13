//
// Created by alqio on 1.9.2022.
//

#include "headers/matlab_processor.h"


MatlabProcessor::MatlabProcessor(const std::string &script_path) {
  std::cout << "Starting matlab" << std::endl;

  matlab = matlab::engine::startMATLAB();
  matlab::engine::connectMATLAB();
  std::cout << "MATLAB started" << std::endl;

  auto path = "addpath " + script_path;
  std::cout << path << std::endl;
  matlab->eval(matlab::engine::convertUTF8StringToUTF16String(path));
}

void MatlabProcessor::init() {
}


std::vector<FpgaEvent> MatlabProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
  matlab::data::ArrayFactory factory;

  auto matlab_data_array = factory.createArray<double>({50, 62});
  auto dim = matlab::data::ArrayDimensions(data.channel_datapoint.size());
  auto matlab_new_sample = factory.createArray<double>(
      {1, data.channel_datapoint.size()},
      data.channel_datapoint.data(),
      data.channel_datapoint.data() + data.channel_datapoint.size()
  );


  std::vector<matlab::data::Array> args({
                                            matlab_data_array,
                                            matlab_new_sample
                                        });

  std::vector<matlab::data::Array> results = matlab->feval(u"MatlabProcessorScript", 2, args);
  std::cout << "feval done" << std::endl;

  auto new_data = results[0];
  auto new_data_typed = (matlab::data::TypedArray<double>) new_data;

  for (auto i: new_data_typed) {
    //std::cout << i << std::endl;
  }

  auto events = results[1];
  auto events_typed = (matlab::data::StructArray) events;

  auto fields = events_typed.getFieldNames();
  std::vector<matlab::data::MATLABFieldIdentifier> field_names(fields.begin(), fields.end());

  for (unsigned event_index = 0; event_index < events_typed.getNumberOfElements(); event_index++) {
    std::cout << "event index " << event_index << std::endl;
    for (auto field: field_names) {
      auto field_name = field.operator std::string();
      std::cout << field_name << ": " << std::endl;
      ///std::cout << field_value[0] << std::endl;

      if (field_name == "channel") {
        matlab::data::TypedArrayRef<uint8_t> field_value = events_typed[event_index][field];
        std::cout << +field_value[0] << std::endl;
      } else if (field_name == "event_type") {
        matlab::data::StringArray field_value = events_typed[event_index][field];
        std::cout << "here" << std::endl;
        auto str = field_value[0].operator std::string();
        std::cout << "casted to str" << std::endl;
        std::cout << str << std::endl;
      } else {
        std::cout << std::endl;
      }
    }
  }

  std::vector<FpgaEvent> ret;
  return ret;
}

int MatlabProcessor::close() {
  std::cout << "finalized matlab" << std::endl;
  matlab::engine::terminateEngineClient();
}