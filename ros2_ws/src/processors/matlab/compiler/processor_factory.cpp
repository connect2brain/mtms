#include "processor_factory.h"

#include "iostream"

extern "C" Processor *create_processor() {
  Processor* p = new Processor();
  return p;
}

extern "C" void destroy(Processor *p) {
  delete p;
}

int main() {
  auto p = create_processor();
  std::cout << p->window_size << std::endl;
  p->init(20);
  std::cout << p->window_size << std::endl;
}