#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import pytest

def test_cyclic_buffer():
    """Tests CyclicBuffer class."""
    from cyclic_buffer import CyclicBuffer

    buffer = CyclicBuffer(4, 2)

    ## Test appending fewer than the maximum number of values

    buffer.append([1, 2], 0.1)
    buffer.append([3, 4], 0.2)
    buffer.append([5, 6], 0.3)

    data, timestamps = buffer.get_buffer()

    assert np.array_equal(data, [
        [0., 0.],
        [1., 2.],
        [3., 4.],
        [5., 6.],
    ])
    assert np.array_equal(timestamps, [0, 0.1, 0.2, 0.3])

    ## Test appending more than the maximum number of values

    buffer.append([7, 8], 0.4)
    buffer.append([9, 10], 0.5)

    data, timestamps = buffer.get_buffer()

    assert np.array_equal(data, [
        [3., 4.],
        [5., 6.],
        [7., 8.],
        [9., 10.],
    ])
    assert np.array_equal(timestamps, [0.2, 0.3, 0.4, 0.5])

    ## Test getting a timerange of values

    # Both endpoints are included

    data, timestamps = buffer.get_timerange(0.2, 0.3)

    assert np.array_equal(data, [
        [3., 4.],
        [5., 6.],
    ])
    assert np.array_equal(timestamps, [0.2, 0.3])

    # Empty result

    data, timestamps = buffer.get_timerange(0.21, 0.29)

    assert data.size == 0
    assert timestamps.size == 0

    # Full buffer

    data, timestamps = buffer.get_timerange(0.1, 0.6)

    assert np.array_equal(data, [
        [3., 4.],
        [5., 6.],
        [7., 8.],
        [9., 10.],
    ])
    assert np.array_equal(timestamps, [0.2, 0.3, 0.4, 0.5])

    ## Test getting the latest datapoint

    datapoint, timestamp = buffer.get_latest()

    assert np.array_equal(datapoint, [9., 10.])
    assert np.array_equal(timestamp, 0.5)
