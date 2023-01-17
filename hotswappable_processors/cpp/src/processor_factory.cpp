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

  auto repeats = 27000;

  for (auto i = 0; i < repeats; i++) {
    std::vector<double> data;
    for (auto j = 0; j < 62; j++) {
      data.push_back(fRand(0, 100));
    }

    std::vector<eeg_sample> events = p->raw_eeg_received(data, 10.0, false);
    //p->samples.print();
    //p->samples.get_buffer();
    //std::cout << "--" << std::endl;
    if (!events.empty()) {
      std::cout << "received event" << std::endl;
    }

  }

}
