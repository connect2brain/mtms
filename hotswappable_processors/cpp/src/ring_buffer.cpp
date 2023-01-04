//
// Created by alqio on 3.1.2023.
//

#include "ring_buffer.h"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;
using namespace std::chrono;

RingBuffer::RingBuffer(unsigned int window_size, unsigned int columns) {
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

void RingBuffer::append(std::vector<double> sample) {
  this->buffer[this->index] = sample;
  this->index = (this->index + 1) % this->window_size;

  if (this->index == 0) {
    this->full = true;
  } else {
    this->nof_elements++;
  }
}

std::vector<double> RingBuffer::operator[](unsigned int i) {
  return this->buffer[(i + this->index) % this->window_size];
}

std::vector<double> RingBuffer::at(unsigned int i) {
  return this->buffer[(i + this->index) % this->window_size];
}

std::vector<std::vector<double>> RingBuffer::get_buffer() {
  std::vector<std::vector<double>> buf;

  if (!this->full) {
    return std::vector<std::vector<double>>(this->buffer.begin(), this->buffer.begin() + index);
  }
  for (unsigned i = this->index; i <= this->nof_elements; i++) {
    buf.push_back(this->at(i));
  }
  for (unsigned i = 0; i < this->index; i++) {
    buf.push_back(this->at(i));
  }

  //return buf;

  auto start = steady_clock::now();

  auto left = std::vector<std::vector<double>>(this->buffer.begin() + index, this->buffer.end());
  auto right = std::vector<std::vector<double>>(this->buffer.begin(), this->buffer.begin() + index);

  auto end = steady_clock::now();
  auto total = duration_cast<microseconds>(end - start);
  std::copy(right.begin(), right.end(), std::back_inserter(left));
  std::cout << "duration: " << total.count() << std::endl;

  return left;
}

void RingBuffer::print() {
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