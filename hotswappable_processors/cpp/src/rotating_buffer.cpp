//
// Created by alqio on 4.1.2023.
//

#include "rotating_buffer.h"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;
using namespace std::chrono;

RotatingBuffer::RotatingBuffer(unsigned int window_size, unsigned int columns) {
  this->window_size = window_size;
  this->columns = columns;
  this->full = false;
  this->index = 0;
  this->nof_elements = 0;

  for (unsigned i = 0; i < this->window_size; i++) {
    std::vector<double> vec(this->columns, 0);
    this->buffer.push_back(vec);
  }
  printf("Window size: %u, columns: %u\n", this->window_size, this->columns);

}

void RotatingBuffer::append(std::vector<double> sample) {
  auto start = steady_clock::now();

  if (!this->full) {
    this->buffer[this->nof_elements] = sample;
    this->nof_elements++;
  } else {
    std::rotate(this->buffer.begin(), this->buffer.begin() + 1, this->buffer.end());
    this->buffer[this->window_size - 1] = sample;
  }

  if (this->nof_elements == this->window_size) {
    this->full = true;
  }


  auto end = steady_clock::now();
  auto total = duration_cast<nanoseconds>(end - start);
  //printf("RotatingBuffer append took %lu ns\n", total.count());
}

std::vector<double> RotatingBuffer::operator[](unsigned int i) {
  return this->buffer[(i + this->index) % this->window_size];
}

std::vector<double> RotatingBuffer::at(unsigned int i) {
  return this->buffer[(i + this->index) % this->window_size];
}

std::vector<std::vector<double>> RotatingBuffer::get_buffer() {
  auto start = steady_clock::now();

  if (!this->full) {
    auto v = std::vector<std::vector<double>>(this->buffer.begin(), this->buffer.begin() + this->nof_elements);
    auto end = steady_clock::now();
    auto total = duration_cast<nanoseconds>(end - start);

    printf("RotatingBuffer not full, get buffer took %lu ns\n", total.count());
    return v;
  }

  auto end = steady_clock::now();
  auto total = duration_cast<nanoseconds>(end - start);

  printf("RotatingBuffer get buffer took %lu ns\n", total.count());
  return this->buffer;
}

void RotatingBuffer::print() {
  auto buf = this->get_buffer();
  auto row_limit = std::min(this->window_size, this->nof_elements);
  for (unsigned row = 0; row < row_limit; row++) {

    std::cout << "[";
    for (unsigned col = 0; col < this->columns; col++) {
      std::cout << buf[row][col] << " ";
    }
    std::cout << "]";

    std::cout << " | ";

    std::cout << "[";

    for (unsigned col = 0; col < this->columns; col++) {
      std::cout << this->buffer[row][col] << " ";
    }
    std::cout << "]" << std::endl;


  }
}