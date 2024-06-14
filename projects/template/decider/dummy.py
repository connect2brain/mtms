import numpy as np

import cpp_bindings


# Override Python's native print() function.
def print(x):
    cpp_bindings.log(str(x))

def print_throttle(x, period):
    cpp_bindings.log_throttle(str(x), period)


LOW_INTENSITY_TARGET = [
    {
        'displacement_x': 0,
        'displacement_y': 0,
        'rotation_angle': 0,
        'intensity': 20,
        'algorithm': 'least_squares',
    },
]

HIGH_INTENSITY_TARGET = [
    {
        'displacement_x': 0,
        'displacement_y': 0,
        'rotation_angle': 0,
        'intensity': 30,
        'algorithm': 'least_squares',
    },
]

PAIRED_PULSE_TARGET = [
    {
        'displacement_x': 0,
        'displacement_y': 0,
        'rotation_angle': 0,
        'intensity': 30,
        'algorithm': 'least_squares',
    },
    {
        'displacement_x': 0,
        'displacement_y': 0,
        'rotation_angle': 0,
        'intensity': 20,
        'algorithm': 'least_squares',
    },
]


TRIGGERS_ENABLED = [
    {
        'enabled': True,
        'delay': 0.0,
    },
    {
        'enabled': True,
        'delay': 0.0,
    },
]

TIME_BUFFER = 0.005  # seconds


class Decider:
    def __init__(self, num_of_eeg_channels, num_of_emg_channels, sampling_frequency):
        self.num_of_eeg_channels = num_of_eeg_channels
        self.num_of_emg_channels = num_of_emg_channels
        self.sampling_frequency = sampling_frequency

        # Various parameters.
        self.trial_interval = 2.0  # seconds
        self.trial_interval_in_samples = int(self.sampling_frequency * self.trial_interval)

        # Initialize state.
        self.sample_count = 0

        # Configure the length of sample window.
        self.sample_window = [-5, 0]

        self.targets = [
            LOW_INTENSITY_TARGET,
#            HIGH_INTENSITY_TARGET,
#            PAIRED_PULSE_TARGET,
        ]

        # Alternate between different target types.
        self.target_type = 0

    def process(self, current_time, timestamps, valid_samples, eeg_samples, emg_samples, current_sample_index, ready_for_trial):
        self.sample_count += 1

        print_throttle(eeg_samples[1, current_sample_index], 1)

        if not ready_for_trial:
            print_throttle("Not ready for trial", 1)
            return

        # Do not proceed if there are invalid samples.
        if not np.all(valid_samples):
            return

        # Otherwise, perform trial once every two seconds.
        if self.sample_count % self.trial_interval_in_samples != 0:
            return

        # Select the target type.
        targets = self.targets[self.target_type]
        self.target_type = (self.target_type + 1) % len(self.targets)

        # Determine the pulse times.
        start_time = current_time + TIME_BUFFER
        if len(targets) == 1:
            pulse_times = [start_time]  # Single pulse.
        else:
            pulse_times = [start_time, start_time + 0.1]  # 100 ms delay between pulses.

        print("Decided at time {:.4f} to perform trial at time {:.4f} with {} target(s)".format(
            current_time,
            start_time,
            len(targets)))

        trial = {
            'targets': targets,
            'pulse_times': pulse_times,
            'triggers': TRIGGERS_ENABLED,
        }
        return {
            'trial': trial,
        }
