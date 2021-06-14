import argparse
import logging
import os
import sys
from typing import Any, List, Tuple

import numpy as np
from numpy.typing import ArrayLike
from mne.io import concatenate_raws, read_raw_edf

from eeg_simulator import EegSimulator

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

eeg_simulator: EegSimulator = EegSimulator()

def parse_paths(datafile_paths: List[str]) -> List[str]:
    """Given a list of paths, either relative or absolute, return the absolute paths.

    Parameters
    ----------
    datafile_paths
        A list of paths to the datafiles. If any one of them is absolute, all the
        files after th will be searched for from a path relative to that one, in
        addition to being searched for as an absolute path and a path relative
        to the current working directory.

    Returns
    -------
        A list of the absolute paths as strings.

    """
    current_path = os.getcwd()
    active_path = current_path
    datafiles = list()
    for path in datafile_paths:
        if os.path.isfile(path):
            datafiles.append(os.path.realpath(path))
            active_path = os.path.dirname(os.path.abspath(path))
        elif os.path.isfile(os.path.join(current_path, path)):
            datafiles.append(os.path.abspath(os.path.join(current_path, path)))
        elif os.path.isfile(os.path.join(active_path, path)):
            datafiles.append(os.path.abspath(os.path.join(active_path, path)))
        else:
            logging.error("Could not find datafile '{}'. Aborting.".format(path))
            sys.exit(1)
    return datafiles

def load_data(datafiles: List[str]) -> Tuple[ArrayLike, List[float], int]:
    """Given a list of paths to the EDF format data files, read the
    files and return the concatenated data.

    Parameters
    ----------
    datafiles
        A list of paths to the data files, in EDF format.

    Returns
    -------
        A tuple consisting of the data, the recording times (in milliseconds), and
        the sampling frequency.

    """
    raws = [read_raw_edf(f, preload=True) for f in datafiles]
    raw = concatenate_raws(raws)

    data: ArrayLike
    times: List[float]
    data, times = raw.get_data(return_times=True)
    data: ArrayLike = np.transpose(data)
    sampling_frequency: float = raw.info['sfreq']

    logging.info(f"Frequency = {sampling_frequency}.")
    logging.info(f"Cycle time = {1.0 / sampling_frequency}.")

    return data, times, sampling_frequency

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Send EEG data over Kafka")
    parser.add_argument('filenames', type=str, help="Files to send over Kafka.", nargs='+')

    args = parser.parse_args()

    datafiles = parse_paths(args.filenames)

    logging.info("Preloading data")

    data: ArrayLike
    times: List[float]
    sampling_frequency: int
    data, times, sampling_frequency = load_data(datafiles)

    # TODO: Publish metadata (EEG channel names, sampling frequency,
    #   something else from raw.info?)

    logging.info("Starting to send data")
    success = eeg_simulator.stream_data_mp(data, sampling_frequency)

    sys.exit(0 if success else 1)
