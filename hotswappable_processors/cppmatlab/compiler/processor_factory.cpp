#include "processor_factory.h"

#include "iostream"

extern "C" MatlabProcessor *create_processor(unsigned int b_window_size,
                                             unsigned short channel_count) {
  auto *p = new MatlabProcessor();
  p->init();
  return p;
}

extern "C" void destroy_processor(MatlabProcessor *p) {
  delete p;
}

double fRand(double fMin, double fMax) {
  double f = (double) rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

int main() {
  MatlabProcessorInterface *processor;
  auto *p = new MatlabProcessor();
  p->init();
  processor = p;

  auto start = std::chrono::high_resolution_clock::now();
  auto stop = std::chrono::high_resolution_clock::now();
  auto total = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  auto repeats = 10000;
  for (auto i = 0; i < repeats; i++) {
    std::vector<double> data;
    for (auto j = 0; j < 62; j++) {
      data.push_back(fRand(0, 100));
    }
    coder::array<matlab_event, 1U> events;
    coder::array<matlab_eeg_sample, 1U> samples;
    p->data_received(data.data(), data.size(), 100, false, events, samples);

    for (auto e = events.begin(); e != events.end(); e++) {
      std::cout << "received event: " << +e->event_type << std::endl;
    }
  }
  coder::array<matlab_event, 1U> events;
  p->end_experiment(events);

}
