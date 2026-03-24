#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>
#include <stdexcept>

#include "mtms_eeg_interfaces/msg/sample.hpp"

template<typename T>
class RingBuffer {
private:
  std::vector<T> buffer;
  size_t capacity;
  size_t current_size;
  size_t head;
  size_t tail;

public:
  RingBuffer() : capacity(0), current_size(0), head(0), tail(0) {}

  /* Reset buffer and set its size. Discards all current data. */
  void reset(size_t size) {
    buffer.resize(size);

    capacity = size;
    current_size = 0;
    head = 0;
    tail = 0;
  }

  /* Appends a message to the buffer, overwriting the oldest message if buffer is full. */
  void append(const T& msg) {
    if (capacity == 0) {
      throw std::runtime_error("RingBuffer capacity is zero; call reset() first");
    }
    buffer[tail] = msg;
    tail = (tail + 1) % capacity;

    if (current_size < capacity) {
      current_size++;
    } else {
      head = (head + 1) % capacity;
    }
  }

  size_t size() const { return current_size; }
  size_t max_size() const { return capacity; }
  bool empty() const { return current_size == 0; }

  /* Returns element i in chronological order (0 = oldest). */
  const T& at(size_t i) const {
    if (i >= current_size) {
      throw std::out_of_range("RingBuffer index out of range");
    }
    return buffer[(head + i) % capacity];
  }

  const T& oldest() const {
    if (empty()) {
      throw std::runtime_error("RingBuffer is empty");
    }
    return buffer[head];
  }

  const T& newest() const {
    if (empty()) {
      throw std::runtime_error("RingBuffer is empty");
    }
    const size_t newest_index = (tail + capacity - 1) % capacity;
    return buffer[newest_index];
  }

  /* Process each element in the buffer using a provided callback function. */
  void process_elements(std::function<void(const T&)> callback) const {
    for (size_t i = 0; i < current_size; ++i) {
      callback(buffer[(head + i) % capacity]);
    }
  }

  /* Queries if the buffer is full. */
  bool is_full() const {
    return current_size == capacity;
  }
};

#endif // RING_BUFFER_H
