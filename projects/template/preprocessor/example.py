import multiprocessing
import time

import numpy as np

import cpp_bindings

# Override Python's native print() function.
def print(x):
    cpp_bindings.log(str(x))


class Preprocessor:
    def __init__(self, num_of_eeg_channels, num_of_emg_channels, sampling_frequency):
        self.num_of_eeg_channels = num_of_eeg_channels
        self.num_of_emg_channels = num_of_emg_channels
        self.sampling_frequency = sampling_frequency

        # Initialize state.
        self.ongoing_pulse_artifact = False
        self.samples_after_pulse = 0
        self.sample_count = 0

        # Configure the length of sample window.
        self.sample_window = [-5, 5]

    def process(self, timestamps, eeg_samples, emg_samples, current_sample_index, pulse_given):
        self.sample_count += 1

        if self.sample_count % 1000 == 0:
            # Do every 1000 samples. Useful for debug prints.
            pass

        if pulse_given:
            self.ongoing_pulse_artifact = True
            self.samples_after_pulse = 0
            print("A pulse was given.")

        # Assuming that an ongoing artifact lasts for 1000 samples; after that, reset the flag.
        if self.ongoing_pulse_artifact:
            self.samples_after_pulse += 1
            if self.samples_after_pulse == 1000:
                self.ongoing_pulse_artifact = False

        eeg_sample_preprocessed = eeg_samples[current_sample_index,:]
        emg_sample_preprocessed = emg_samples[current_sample_index,:]

        return {
            'eeg_sample': eeg_sample_preprocessed,
            'emg_sample': emg_sample_preprocessed,
            'valid': not self.ongoing_pulse_artifact,
        }
