#include "processor_factory.h"

#include "iostream"

extern "C" MatlabProcessor *create_processor(unsigned int b_window_size,
                                             unsigned short channel_count) {
  auto *p = new MatlabProcessor();
  p->init(b_window_size, channel_count);
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