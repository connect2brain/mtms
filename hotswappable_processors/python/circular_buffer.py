"""
Adapted from https://stackoverflow.com/a/40784706/6215543
"""


class CircularBuffer(object):
    def __init__(self, size, data=None):
        """Initialization"""
        if data is None:
            data = []
        self.index = 0
        self.size = size
        self._data = list(data)[-size:]

    def data(self):
        return self._data[self.index:] + self._data[:self.index]

    def append(self, value):
        """Append an element"""
        if len(self._data) == self.size:
            self._data[self.index] = value
        else:
            self._data.append(value)
        self.index = (self.index + 1) % self.size

    def __getitem__(self, key):
        """Get element by index, relative to the current index"""
        if len(self._data) == self.size:
            return self._data[(key + self.index) % self.size]
        else:
            return self._data[key]

    def __repr__(self):
        """Return string representation"""
        return (self._data[self.index:] + self._data[:self.index]).__repr__() + ' (' + str(
            len(self._data)) + '/{} items)'.format(self.size)
