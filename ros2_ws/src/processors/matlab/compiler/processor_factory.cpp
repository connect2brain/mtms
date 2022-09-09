#include "processor_factory.h"

#include "iostream"

extern "C" MatlabProcessorInterface *create_processor(unsigned int b_window_size,
                                                      unsigned short channel_count) {
  auto *p = new MatlabProcessorInterface();
  p->init(b_window_size, channel_count);
  return p;
}

extern "C" void destroy(MatlabProcessorInterface *p) {
  delete p;
}

int main() {
  auto p = create_processor(20, 62);
  std::cout << p->window_size << std::endl;
  std::cout << p->window_size << std::endl;
}