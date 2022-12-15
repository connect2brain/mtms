"""
Adapted from https://stackoverflow.com/a/40784706/6215543
"""

import numpy as np


class CircularBuffer(object):
    def __init__(self, window_size, columns, dtype=float):
        self.index = 0
        self.size = window_size
        self.shape = (window_size, columns)
        self.buffer = np.zeros(shape=self.shape, dtype=dtype)

    def get_buffer(self):
        return self.buffer[self.index:] + self.buffer[:self.index]

    def append(self, value):
        self.buffer[self.index] = value
        self.index = (self.index + 1) % self.size

    def __getitem__(self, key):
        return self.buffer[(key + self.index) % self.size]
