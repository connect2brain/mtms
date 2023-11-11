import numpy as np

import cpp_bindings


# Override Python's native print() function.
def print(x):
    cpp_bindings.log(str(x))


class Decider:
    def __init__(self, num_of_eeg_channels, num_of_emg_channels, sampling_frequency):
        self.num_of_eeg_channels = num_of_eeg_channels
        self.num_of_emg_channels = num_of_emg_channels
        self.sampling_frequency = sampling_frequency

        # Various parameters.
        self.trigger_interval_in_samples = self.sampling_frequency * 4

        # Initialize state.
        self.sample_count = 0

        # Configure the length of sample window.
        self.sample_window = [-5, 0]

    def process(self, current_time, timestamps, valid_samples, eeg_samples, emg_samples, current_sample_index, ready_for_trigger):
        self.sample_count += 1

        if self.sample_count % 1000 == 0:
            # Do every 1000 samples. Useful for debug prints.
            pass

        # Do not proceed if there are invalid samples.
        if not np.all(valid_samples):
            return {
                'send_trigger': False,
                'sensory_stimulus': None,
            }

        # Otherwise, send trigger once every second.
        if self.sample_count % self.trigger_interval_in_samples != 0:
            return {
                'send_trigger': False,
                'sensory_stimulus': None,
            }

        return {
            'send_trigger': ready_for_trigger,
            'sensory_stimulus': {
                'time': current_time,
                'state': 0,
                'parameter': 1,
                'duration': 10.0
            }
        }
