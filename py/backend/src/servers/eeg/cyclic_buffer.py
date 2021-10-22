#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np

from typing import Tuple
from numpy.typing import ArrayLike

class CyclicBuffer:
    """A buffer that stores a fixed number of the previous values,
    with a numeric timestamp attached to each value.

    Supports various get operations and appending to the end.

    The buffer is initialized with NaN values and -inf timestamps.
    """

    def __init__(self, length: int, dim: int) -> None:
        """Initialize a buffer with a given length and data dimensionality.

        Parameters
        ----------
        length
            The maximum number of values stored in the buffer.
        dim
            The dimensionality of each value, i.e., the number of elements
            that the value consists of.
        """
        self._length: int = length
        self._dim: int = dim
        self._data: ArrayLike = np.full((length, dim), np.nan)
        self._timestamps: ArrayLike = np.full(length, -np.inf)
        self._i: int = 0

    def append(self, value: ArrayLike, timestamp: float) -> None:
        """Append a value with a numeric timestamp.

        Parameters
        ----------
        value
            An array of the length that equals the dimensionality of the
            buffer.
        timestamp
            A timestamp attached to the value.

        Notes
        -----
        The timestamps must be increasing, that is, an appended timestamp
        must not be smaller than the previous timestamp.
        """
        assert len(value) == self._dim
        if not np.isnan(self._timestamps[self._i - 1]):
            assert timestamp >= self._timestamps[self._i - 1]

        self._data[self._i] = value
        self._timestamps[self._i] = timestamp
        self._i = (self._i + 1) % self._length

    def get_latest(self) -> Tuple[float, float]:
        """Return the latest datapoint and timestamp appended.

        Returns
        -------
            The value of the latest datapoint.

            The timestamp of the latest datapoint.
        """
        return self._data[self._i - 1], self._timestamps[self._i - 1]

    def get_buffer(self) -> Tuple[ArrayLike, ArrayLike]:
        """Return all data and timestamps stored in the buffer.

        Returns
        -------
            A float array of the data values.

            A float array of the corresponding timestamps.

        Notes
        -----
        The data are returned in an increasing time order.
        """
        inds: ArrayLike = np.concatenate([np.arange(self._i, self._length),
                                          np.arange(0, self._i)])
        return self._data[inds], self._timestamps[inds]

    def get_timerange(self, t0: float, t1: float) -> Tuple[ArrayLike, ArrayLike]:
        """Return the data and their timestamps between two given timestamps.

        Parameters
        ----------
        t0, t1
            The start and end timestamps, respectively.

        Returns
        -------
            A float array of the data values.

            A float array of the corresponding timestamps.

        Notes
        -----
        The endpoints of the time range are included.
        The result is returned in an increasing time order.
        """
        data: ArrayLike
        timestamps: ArrayLike
        data, timestamps = self.get_buffer()
        inds: ArrayLike = np.logical_and(timestamps >= t0, timestamps <= t1)
        return data[inds], timestamps[inds]
