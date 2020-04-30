#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import dotenv
import logging
import pytest
dotenv.load_dotenv()


def test_edfreader():
    """Tests EDFReader class.
    """
    from readersender import (ReaderSender, Reader)
    from edfreader import EDFReader

    test_edf_file = os.path.join("..", os.getenv('DATASET_PATH'), os.getenv('EDF_TEST_FILE'))
    if not os.path.isfile(test_edf_file):
        raise FileNotFoundError("File '{}' not found. Cannot test reading EDF. Perhaps you need to set 'DATASET_PATH' and 'EDF_TEST_FILE' in .env.".format(test_edf_file))

    reader = EDFReader(edf_file=test_edf_file, logger=logging.getLogger('test_edfreader'))
    assert isinstance(reader, EDFReader)
    assert isinstance(reader, Reader)
    assert isinstance(reader, ReaderSender)

    assert not reader.connected
    with pytest.raises(RuntimeError):
        reader.read()   # NOTE: This should fail as we are not connected

    reader.connect()
    assert reader.connected

    data_len = reader.data_len
    assert data_len > 0

    data_f = reader.data_frequency
    assert data_f > 0

    for _ in range(data_len):
        assert reader.read() is not None

    assert reader.read() is None   # Iteration has stopped, so it should be None
    assert reader.read() is None   # None should repeat

    assert reader.connected
    reader.disconnect()
    assert not reader.connected

    with pytest.raises(RuntimeError):
        reader.data

    with pytest.raises(RuntimeError):
        reader.data_len

    with pytest.raises(RuntimeError):
        reader.data_frequency
