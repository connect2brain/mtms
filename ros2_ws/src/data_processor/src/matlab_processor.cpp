//
// Created by alqio on 1.9.2022.
//

#include "headers/matlab_processor.h"

MatlabProcessor::MatlabProcessor(std::string script_path) {
  /*
  matlab = matlab::engine::startMATLAB();
  matlab::engine::connectMATLAB();
  std::cout << "MATLAB started" << std::endl;

  std::string path = "sometext" + script_path + "sometext";
  std::cout << path << std::endl;
  matlab->eval(u"addpath '/home/alqio/workspace/mtms/ros2_ws/src/processors'");
  matlab->eval(u"processor = Processor");
  //matlab->eval(u"processor.new_datapoint(")

  //matlab::data::Array a = matlab->feval(u"gcd");
  processor_instance = matlab->getVariable(u"processor");

  auto add_simple = matlab->getProperty(processor_instance, u"add_simple");
  auto v = add_simple[0];

  matlab::data::ArrayFactory factory;
  std::vector<matlab::data::Array> args({factory.createScalar<int32_t>(30)});

  auto cresult = matlab->feval<int>(u"processor.add_simple");
  std::cout << "cresult: " << cresult << std::endl;
  //auto v = cresult[0];
  //std::cout << "cresult: " << v << std::endl;
*/
}

void MatlabProcessor::init() {
}


std::vector<FpgaEvent> MatlabProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {
}

int MatlabProcessor::close() {
  std::cout << "finalized matlab" << std::endl;
  matlab::engine::terminateEngineClient();
}