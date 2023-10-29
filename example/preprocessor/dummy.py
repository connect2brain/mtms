import multiprocessing
import time

import numpy as np

import preprocessor_bindings

# Override Python's native print() function.
def print(x):
    preprocessor_bindings.log(str(x))


class Preprocessor:
    def __init__(self, num_of_eeg_channels, num_of_emg_channels, sampling_frequency):
        self.num_of_eeg_channels = num_of_eeg_channels
        self.num_of_emg_channels = num_of_emg_channels
        self.sampling_frequency = sampling_frequency

        # Initialize state.
        self.ongoing_pulse_artifact = False
        self.samples_after_pulse = 0

        # Configure the length of sample window.
        self.sample_window = [-5, 5]

    def process(self, timestamps, eeg_data, emg_data, current_sample_index, pulse_given):
        if pulse_given:
            self.ongoing_pulse_artifact = True
            self.samples_after_pulse = 0
            print("A pulse was given.")

        # Assuming that an ongoing artifact lasts for 1000 samples; after that, reset the flag.
        if self.ongoing_pulse_artifact:
            self.samples_after_pulse += 1
            if self.samples_after_pulse == 1000:
                self.ongoing_pulse_artifact = False

        eeg_data_preprocessed = eeg_data[current_sample_index,:]
        emg_data_preprocessed = emg_data[current_sample_index,:]

        return {
            'eeg_data': eeg_data_preprocessed,
            'emg_data': emg_data_preprocessed,
            'valid': not self.ongoing_pulse_artifact,
        }
