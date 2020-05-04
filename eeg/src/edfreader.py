#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import pathlib
import logging
import numpy as np
import mne.io
from readersender import Reader
from readersender.helpers import (only_connected, only_disconnected)


class EDFReader(Reader):
    """A Reader class the read EDF data files row-by-row.
    """
    def __init__(self, edf_file, *args, **kwargs):
        """Initializes an EDFReader with given file.

        Parameters
        ----------
        edf_file : path or str_like
            Path to EDF file.
        """
        super().__init__(*args, **kwargs)

        edf_path = pathlib.Path(edf_file)
        if not edf_path.is_file():
            raise FileNotFoundError("File '{}' not found.".format(edf_file))
        self._edf_path = edf_path

        self._edf_data = None
        self._edf_fs = 0
        self._edf_len = 0
        self._edf_iter = None

    @property
    def edf_file(self):
        """The EDF file attached to this reader.
        """
        return str(self._edf_path)

    @edf_file.setter
    def edf_file(self, edf_file):
        """Sets the EDF file to read.

        Parameters
        ----------
        edf_file : path or str_like
            Path to EDF file.

        Notes
        -----
        If reader will be disconnected after changing the file.
        """
        if self.connected:
            self.disconnect()

        edf_path = pathlib.Path(edf_file)
        if not edf_path.is_file():
            raise FileNotFoundError("File '{}' not found.".format(edf_file))
        self._edf_path = edf_path

        self._edf_data = None
        self._edf_fs = 0
        self._edf_len = 0
        self._edf_iter = None

    @property
    @only_connected(action='raise')
    def data(self):
        """Returns the loaded EDF data.

        """
        return self._edf_data

    @property
    @only_connected(action='raise')
    def data_len(self):
        """Returns the size of the loaded EDF file.

        """
        return self._edf_len

    @property
    @only_connected(action='raise')
    def data_frequency(self):
        """Returns the frequency of the loaded EDF file.

        """
        return self._edf_fs

    @property
    def connected(self):
        """Checks if EDF file is read to memory.

        """
        return self._edf_data is not None

    @only_disconnected()
    def connect(self):
        """Reads the EDF file to memory.

        """
        # Read the data on at a time
        self.log("Preloading data", logging.INFO)
        raw = mne.io.read_raw_edf(self._edf_path, preload=True)
        data = raw.get_data(return_times=False)
        self._edf_data = np.transpose(data)
        self._edf_fs = raw.info['sfreq']
        self._edf_len = len(self._edf_data)
        self._edf_iter = None
        self.log(f"Frequency = {self._edf_fs}; Cycle time = {1.0 / self._edf_fs}.",
                 logging.INFO)

    @only_connected()
    def disconnect(self):
        """Deletes the EDF file from memory.

        """
        self._edf_data = None
        self._fs = 0
        self._edf_len = 0
        self._edf_iter = None

    @only_connected()
    def reset(self):
        """Resets the data to the beginning.

        """
        if self._edf_iter is not None:
            # Delete the iterator
            self._edf_iter = None

    @only_connected(action='raise')
    def read(self):
        """Reads a single line from EDF file.

        Yields
        ------
        np.array
            A single line from EDF file.
        """
        if self._edf_iter is None:
            # Initialize an iterator
            self._edf_iter = iter(self._edf_data)

        try:
            return next(self._edf_iter)
        except StopIteration:
            return None
