#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np

class CyclicBuffer:
    """A buffer that stores a fixed number of the previous values,
    with a numeric timestamp attached to each value.

    Supports various get operations and appending to the end.

    The buffer is initialized with NaN values and timestamps."""

    def __init__(self, length, dim):
        """Initialize a buffer with a given length and data dimensionality.

        Parameters
        ----------
        length: int
            The maximum number of values stored in the buffer.
        dim: int
            The dimensionality of each value, i.e., the number of elements
            that the value consists of.
        """
        self._length = length
        self._dim = dim
        self._data = np.full((length, dim), np.nan)
        self._timestamps = np.full(length, np.nan)
        self._i = 0

    def append(self, value, timestamp):
        """Append a value with a numeric timestamp.

        Note that the timestamps must be increasing, that is, an appended
        timestamp must not be smaller than the previous timestamp.

        Parameters
        ----------
        value: array_like
            An array of the length that equals the dimensionality of the
            buffer.
        timestamp: number
            A timestamp attached to the value.
        """
        assert len(value) == self._dim
        if not np.isnan(self._timestamps[self._i - 1]):
            assert timestamp >= self._timestamps[self._i - 1]

        self._data[self._i] = value
        self._timestamps[self._i] = timestamp
        self._i = (self._i + 1) % self._length

    def get_latest(self):
        """Return the latest datapoint and timestamp appended."""
        return self._data[self._i - 1], self._timestamps[self._i - 1]

    def get_buffer(self):
        """Return all data and timestamps stored in the buffer, in increasing
        time order.
        """
        inds = np.concatenate([np.arange(self._i, self._length),
                               np.arange(0, self._i)])
        return self._data[inds], self._timestamps[inds]

    def get_timerange(self, t0, t1):
        """Return the data and their timestamps stored in the buffer
        between two given timestamps.

        The endpoints are included, and the result is returned in increasing
        time order.

        Parameters
        ----------
        t0, t1: number
            The start and end timestamps, respectively.
        """
        data, timestamps = self.get_buffer()
        inds = np.logical_and(timestamps >= t0, timestamps <= t1)
        return data[inds], timestamps[inds]
