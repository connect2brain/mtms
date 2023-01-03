//
// Created by alqio on 3.1.2023.
//

#include "ring_buffer.h"
#include <iostream>

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

std::vector<std::vector<double>> RingBuffer::get_buffer() {
  if (!this->full) {
    return std::vector<std::vector<double>>(this->buffer.begin(), this->buffer.begin() + index);
  }
  auto left = std::vector<std::vector<double>>(this->buffer.begin() + index, this->buffer.end());
  auto right = std::vector<std::vector<double>>(this->buffer.begin(), this->buffer.begin() + index);
  std::copy(right.begin(), right.end(), std::back_inserter(left));
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