//
// Created by alqio on 4.1.2023.
//

#ifndef PROCESSOR_FACTORY_ROTATING_BUFFER_H
#define PROCESSOR_FACTORY_ROTATING_BUFFER_H

#include <vector>

class RotatingBuffer {
public:
  RotatingBuffer(unsigned window_size, unsigned columns);

  std::vector<std::vector<double>> get_buffer();

  void append(std::vector<double> sample);

  std::vector<double> operator[](unsigned i);

  std::vector<double> at(unsigned i);

  void print();

private:
  unsigned window_size;
  unsigned columns;
  std::vector<std::vector<double>> buffer;
  bool full;
  unsigned index;
  unsigned nof_elements;
};


#endif //PROCESSOR_FACTORY_ROTATING_BUFFER_H
