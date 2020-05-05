#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import pytest

def test_cyclic_buffer():
    """Tests CyclicBuffer class.

    """
    from cyclic_buffer import CyclicBuffer

    buffer = CyclicBuffer(4, 2)

    ## Test appending fewer than the maximum number of values

    buffer.append([1, 2], 0.1)
    buffer.append([3, 4], 0.2)
    buffer.append([5, 6], 0.3)

    data, timestamps = buffer.get_buffer()

    np.testing.assert_equal(data, [
        [np.nan, np.nan],
        [1., 2.],
        [3., 4.],
        [5., 6.],
    ])
    np.testing.assert_equal(timestamps, [np.nan, 0.1, 0.2, 0.3])

    ## Test appending more than the maximum number of values

    buffer.append([7, 8], 0.4)
    buffer.append([9, 10], 0.5)

    data, timestamps = buffer.get_buffer()

    np.testing.assert_equal(data, [
        [3., 4.],
        [5., 6.],
        [7., 8.],
        [9., 10.],
    ])
    np.testing.assert_equal(timestamps, [0.2, 0.3, 0.4, 0.5])

    ## Test getting a timerange of values

    # Both endpoints are included

    data, timestamps = buffer.get_timerange(0.2, 0.3)

    np.testing.assert_equal(data, [
        [3., 4.],
        [5., 6.],
    ])
    np.testing.assert_equal(timestamps, [0.2, 0.3])

    # Empty result

    data, timestamps = buffer.get_timerange(0.21, 0.29)

    assert data.size == 0
    assert timestamps.size == 0

    # Full buffer

    data, timestamps = buffer.get_timerange(0.1, 0.6)

    np.testing.assert_equal(data, [
        [3., 4.],
        [5., 6.],
        [7., 8.],
        [9., 10.],
    ])
    np.testing.assert_equal(timestamps, [0.2, 0.3, 0.4, 0.5])

    ## Test getting the latest datapoint

    datapoint, timestamp = buffer.get_latest()

    np.testing.assert_equal(datapoint, [9., 10.])
    np.testing.assert_equal(timestamp, 0.5)

    ## Test wrong input

    with pytest.raises(ValueError):
        buffer.append([[11, 12], [3, 4]], 0.5)   # Wrong data shape
    with pytest.raises(ValueError):
        buffer.append([13, 13], [0.6, 0.7])   # Wrong timestamp length
    with pytest.raises(ValueError):
        buffer.append([[15, 16], [17, 18]], [0.8, 0.9])   # Wrong timestamp length
    with pytest.raises(AssertionError):
        buffer.append([19, 20, 21], 1.0)   # Wrong data input length
    with pytest.raises(AssertionError):
        buffer.append([22, 23], 0.4)   # Timestamp smaller than already registered timestamp

    ## Test buffer overloading
    for i, ts in enumerate(np.arange(0.6, 100000, 0.1)):
        buffer.append([2*i, 2*i+1], ts)

    data, timestamps = buffer.get_buffer()
    np.testing.assert_equal(data, [
        [2*(i-3), 2*(i-3)+1],
        [2*(i-2), 2*(i-2)+1],
        [2*(i-1), 2*(i-1)+1],
        [2*i, 2*i+1],
    ])
    assert np.allclose(timestamps, [
        ts-(3*0.1), ts-(2*0.1), ts-(1*0.1), ts
    ])

