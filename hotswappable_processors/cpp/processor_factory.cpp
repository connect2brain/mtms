#include "processor_factory.h"

#include "iostream"

extern "C" Processor *create_processor() {
  auto *p = new Processor();
  return p;
}

extern "C" void destroy_processor(Processor *p) {
  delete p;
}

double fRand(double fMin, double fMax) {
  double f = (double) rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

int main() {
  Processor *processor;
  auto *p = new Processor();
  processor = p;

  auto start = std::chrono::high_resolution_clock::now();
  auto stop = std::chrono::high_resolution_clock::now();
  auto total = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);


}
