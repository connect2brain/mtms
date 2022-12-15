from collections import deque
import numpy as np


class Buffer:
    def __init__(self, window_size):
        self.window_size = window_size
        self.buffer = deque(np.array([]), self.window_size)

    def append(self, v):
        self.buffer.append(v)

    def get_buffer(self):
        return np.array([v for v in self.buffer])
