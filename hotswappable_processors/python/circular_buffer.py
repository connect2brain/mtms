import numpy as np


class CircularBuffer(object):
    def __init__(self, window_size, columns, dtype=float):
        self.index = 0
        self.size = window_size
        self.shape = (window_size, columns)
        self.buffer = np.zeros(shape=self.shape, dtype=dtype)
        self.full = False

    def get_buffer(self):
        if not self.full:
            return self.buffer[:self.index]

        left = self.buffer[self.index:]
        right = self.buffer[:self.index]

        return np.concatenate((left, right), axis=0)

    def append(self, value):
        self.buffer[self.index] = value
        self.index = (self.index + 1) % self.size

        if self.index == 0:
            self.full = True

    def __getitem__(self, key):
        return self.buffer[(key + self.index) % self.size]
