import argparse
import logging
import os
import sys
from typing import List

import numpy as np
from mne.io import concatenate_raws, read_raw_edf

from eeg_simulator import EegSimulator
from mtms.kafka.kafka import Kafka

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

eeg_simulator: EegSimulator = EegSimulator()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Send EEG data over Kafka")
    parser.add_argument('filenames', type=str, help="Files to send over Kafka.", nargs='+')

    args = parser.parse_args()

    #datafile_paths = [os.path.join(os.getcwd(), path) for path in args.filenames]
    datafile_paths = args.filenames
    # Check for all files
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

    # Read the data on at a time
    logging.info("Preloading data")
    raws = [read_raw_edf(f, preload=True) for f in datafiles]
    raw = concatenate_raws(raws)

    data, times = raw.get_data(return_times=True)
    data = np.transpose(data)
    sampling_frequency = raw.info['sfreq']
    logging.info(f"Frequency = {sampling_frequency}.")
    logging.info(f"Cycle time = {1.0 / sampling_frequency}.")

    # TODO: Publish metadata (EEG channel names, sampling frequency,
    #   something else from raw.info?)

    logging.info("Starting to send data")
    success = eeg_simulator.stream_data_mp(data, sampling_frequency)

    sys.exit(0 if success else 1)
