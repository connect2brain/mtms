"""
Adapted from https://stackoverflow.com/a/40784706/6215543
"""


class CircularBuffer(object):
    def __init__(self, size):
        self.index = 0
        self.size = size
        self.buffer = []

    def get_buffer(self):
        return self.buffer[self.index:] + self.buffer[:self.index]

    def append(self, value):
        if len(self.buffer) == self.size:
            self.buffer[self.index] = value
        else:
            self.buffer.append(value)
        self.index = (self.index + 1) % self.size

    def __getitem__(self, key):
        if len(self.buffer) == self.size:
            return self.buffer[(key + self.index) % self.size]
        else:
            return self.buffer[key]
