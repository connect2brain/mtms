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
  create_processor *create_processor_func = (create_processor *) dlsym(processor_factory, "create_processor");
  destroy *destroy_processor_func = (destroy *) dlsym(processor_factory, "destroy");


  if (!create_processor_func) {
    std::cerr << "Cannot load create_processor_func symbols: " << dlerror() << std::endl;
  }
  if (!destroy_processor_func) {
    std::cerr << "Cannot load destroy_processor_func symbols: " << dlerror() << std::endl;
  }
  auto inner_processor = create_processor_func();

  std::cout << "inner processor: " << inner_processor << std::endl;

  std::vector<double> sample;
  for (auto i = 0; i < 62; i++) {
    sample.push_back(ffRand(0, 100));
  }
  std::cout << "window size before init" << std::endl;
  std::cout << inner_processor->window_size << std::endl;

  inner_processor->init(20);
  std::cout << "window size after init" << std::endl;
  std::cout << inner_processor->window_size << std::endl;

  std::cout << "data size: " << inner_processor->data.size() << std::endl;
  std::cout << "a: ";
  auto a = inner_processor->data.at(3,3);
  std::cout << a << std::endl;
  std::cout << "data: " << inner_processor->data.at(1,2) << std::endl;
  inner_processor->data_received(sample.data());
  inner_processor->data_received(sample.data());
  std::cout << inner_processor->data.data()[0] << std::endl;

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
