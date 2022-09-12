#include "processor_factory.h"

#include "iostream"

extern "C" MatlabProcessor *create_processor(unsigned int b_window_size,
                                             unsigned short channel_count) {
  std::cout << "in processor_factory create_processor " << std::endl;
  auto *p = new MatlabProcessor();
  p->init(b_window_size, channel_count);
  std::cout << "in processor_factory create_processor window size " << p->window_size << std::endl;
  std::cout << "in processor_factory create_processor data at" << p->data.at(0, 1) << std::endl;
  return p;
}

extern "C" void destroy_processor(MatlabProcessor *p) {
  delete p;
}


int main() {
  MatlabProcessorInterface *processor;
  auto *p = new MatlabProcessor();
  p->init(20, 50);

  processor = p;

}