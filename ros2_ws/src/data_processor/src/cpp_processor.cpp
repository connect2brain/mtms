//
// Created by alqio on 1.9.2022.
//

#include "headers/cpp_processor.h"
#include "headers/cpp_processor_interface.h"

double ffRand(double fMin, double fMax) {
  double f = (double) rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

CPPProcessor::CPPProcessor(std::string script_path) {
  processor_factory = dlopen(script_path.c_str(), RTLD_NOW);
  if (processor_factory == 0) {
    std::cerr << "Cannot load processor_factory: " << dlerror() << std::endl;
  }
  std::cout << "loaded factory" << std::endl;
  auto *create_processor_func = (create_processor *) dlsym(processor_factory, "create_processor");
  auto *destroy_processor_func = (destroy *) dlsym(processor_factory, "destroy");


  if (!create_processor_func) {
    std::cerr << "Cannot load create_processor_func symbols: " << dlerror() << std::endl;
  }
  if (!destroy_processor_func) {
    std::cerr << "Cannot load destroy_processor_func symbols: " << dlerror() << std::endl;
  }
  auto inner_processor = create_processor_func(20, 62);

  std::cout << "inner processor: " << inner_processor << std::endl;

  std::vector<double> sample;
  for (auto i = 0; i < 62; i++) {
    sample.push_back(ffRand(0, 100));
  }
  std::cout << "window size: " << inner_processor->window_size << std::endl;

  std::cout << "data size: " << inner_processor->data.size() << std::endl;
  auto a = inner_processor->data.at(3, 3);
  std::cout << "a: " << a << std::endl;

  coder::array<stimulation_pulse_event, 1U> data;
  std::vector<stimulation_pulse_event> data_vector;

  inner_processor->data_received(sample.data(), data);
  inner_processor->data_received(sample.data(), data);
  std::cout << +data.at(0).channel << std::endl;

}

std::vector<FpgaEvent> CPPProcessor::data_received(mtms_interfaces::msg::EegDatapoint data) {

}

void CPPProcessor::init() {

}

int CPPProcessor::close() {
  if (processor_factory != nullptr) {
    dlclose(processor_factory);
  }
  return 0;
}
