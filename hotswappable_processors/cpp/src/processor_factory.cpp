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
  ProcessorInterface *processor;
  auto *p = new Processor();
  processor = p;

  auto repeats = 17;

  for (auto i = 0; i < repeats; i++) {
    std::vector<double> data;
    for (auto j = 0; j < 62; j++) {
      //data.push_back(fRand(0, 100));
    }
    data.push_back(i + 1);
    data.push_back(i + 1);

    std::vector<fpga_event> events = p->data_received(data, 10.0, false);
    p->samples.print();

  }

}
