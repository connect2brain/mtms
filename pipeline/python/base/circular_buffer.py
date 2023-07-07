import numpy as np


class CircularBuffer(object):
    def __init__(self, window_size, columns, dtype=float):
        self._index = 0
        self._size = window_size
        self._shape = (window_size, columns)
        self._buffer = np.zeros(shape=self._shape, dtype=dtype)

        self.is_full = False

    def get_buffer(self):
        if not self.is_full:
            return self._buffer[:self._index]

        left = self._buffer[self._index:]
        right = self._buffer[:self._index]

        return np.concatenate((left, right), axis=0)

    def append(self, value):
        self._buffer[self._index] = value
        self._index = (self._index + 1) % self._size

        if self._index == 0:
            self.is_full = True

    def __getitem__(self, key):
        return self._buffer[(key + self._index) % self.size]
