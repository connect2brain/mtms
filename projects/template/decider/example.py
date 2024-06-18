# An example decider that alternates between different target types, all targeted
# at the same location (x = 0, y = 0, rotation = 0). The target types are as follows:
#
#   - Single pulse with low intensity (20 V/m).
#   - Single pulse with high intensity (30 V/m)
#   - Paired pulse with high intensity followed by low intensity.
#
# A trial with one of the above target types is performed once every two seconds.
#
# Note that if you want to perform trials more frequently, you need to reduce the value
# of MINIMUM_INTERTRIAL_INTERVAL in the environment configuration file (e.g., sites/Aalto/.env
# in the repository root directory) and restart the pipeline (e.g., by running `restart` on the
# command line), as 2.0 seconds is the default minimum intertrial interval for safety reasons.


# Native Python modules and other third-party libraries can be imported here. Of third-party libraries,
# the following are currently available:
#
#  - numpy
#  - scipy
#  - scikit-learn
#  - statsmodels
#  - mneflow
#
# If more third-party libraries are needed, they can be installed by adding them in
# ros2_ws/src/pipeline/decider/Dockerfile, and running 'build' on the command line after
# modifying the file.

import multiprocessing
import time

import numpy as np

# 'print' and 'print_throttle' are custom functions that log messages to the decider log.
# 'print_throttle' function logs messages to the console at most once every 'period' seconds.
#
# The decider log can be accessed from the command line by running: `log decider`. See `log --help`
# for more information on how to use the `log` command.
#
# Usage of 'print' and 'print_throttle' functions:
#
#  print("Hello, world!")
#  print_throttle("This message will be printed at most once every second.")
#  print_throttle("This message will be printed at most once every 0.5 seconds.", period=0.5)
#
# Note that the 'print' function overrides Python's built-in 'print' function, so you can use it
# in the same way as the built-in 'print' function.
#
from common.utils import print, print_throttle


# Define a few target types.
#
# Each target is a list of dictionaries, each dictionary with the following keys:
#  - 'displacement_x': The x-coordinate of the target in millimeters (between -15 and 15).
#  - 'displacement_y': The y-coordinate of the target in millimeters (between -15 and 15).
#  - 'rotation_angle': The rotation angle of the target in degrees (between 0 and 359).
#  - 'intensity': The intensity of the target in V/m.
#  - 'algorithm': The algorithm to use for computing the capacitor voltages based on the target.
#                 The available options are:
#
#     - 'least_squares': Use the least squares algorithm.
#     - 'genetic': Use the genetic algorithm.
#
# A single-pulse target consists of a single dictionary, while a paired-pulse target consists of two dictionaries in a list.

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


# The mTMS device allows coinciding the TMS pulse with one or two output triggers.
# The triggers can be configured to be enabled or disabled and to have a delay before activation.
#
# Each trigger is a dictionary with the following keys:
#
# - 'enabled': A boolean indicating whether the trigger is enabled.
# - 'delay': The delay in seconds before the trigger is activated. The delay can be 0.0, positive,
#            or negative. A positive delay means that the trigger is activated after the pulse,
#            while a negative delay means that the trigger is activated before the pulse.
TRIGGERS = [
    {
        'enabled': True,
        'delay': 0.0,
    },
    {
        'enabled': True,
        'delay': 0.0,
    },
]

# The minimum time between the currently processed EEG/EMG samples and the pulse, in seconds. If it is too
# short, the pulse may not be performed due to the end-to-end latency of the pipeline. The overall latency
# includes the following components:
#
# - The time it takes for the EEG device to filter and send the data.
# - The time it takes for the pipeline to preprocess the data (see preprocessor).
# - The time it takes for the decider to make a decision. This is greatly affected by the complexity
#   of the decision-making algorithm.
#
# In practice, 5 ms is a good value to start with when the pipeline does not do any significant processing.
# The delay should be adjusted based on the pipeline's processing time.
#
# Note that in this example, the pulse is timed exactly at the minimum delay, but in real applications,
# the algorithm implemented in the decider can also time it farther in the future.
MINIMUM_DELAY_BEFORE_PULSE = 0.050  # seconds


class Decider:
    # When the decider is initialized, the number of EEG and EMG channels and the sampling frequency are automatically
    # provided by the pipeline. They are determined based on how the EEG device is configured.
    def __init__(self, num_of_eeg_channels, num_of_emg_channels, sampling_frequency):

        # It is a good practice to store the number of EEG and EMG channels and the sampling frequency for later use.
        self.num_of_eeg_channels = num_of_eeg_channels
        self.num_of_emg_channels = num_of_emg_channels
        self.sampling_frequency = sampling_frequency

        # Define an example value for the intertrial interval in seconds.
        self.intertrial_interval = 2.0  # seconds

        # Compute the intertrial interval in samples, as it is more convenient to use that in the code.
        self.intertrial_interval_in_samples = int(self.sampling_frequency * self.intertrial_interval)

        # Initialize state variable to keep track of the number of samples processed so far.
        self.sample_count = 0

        # The pipeline automatically manages a buffer of the previous EEG and EMG samples. When initialized,
        # the decider can provide pipeline with information about the desired length of the sample window,
        # done by setting the 'sample_window' attribute (see below).
        #
        # The sample window is defined as a list with two elements: the first element is the earliest sample
        # kept in the buffer, and the second element is the latest sample kept in the buffer, relative to 0,
        # which is the current sample.
        #
        # Examples:
        #
        #   - If the sample window is [-5, 0], the buffer will keep the latest 5 samples and the current
        #     sample, for a total of 6 samples. The buffer will discard older samples.
        #
        #   - Using a sampling frequency of 1000 Hz, defining the sample window as [-999, 0] will keep the
        #     last second of samples in the buffer.
        #
        #   - If the sample window is [0, 0], the buffer will keep only the current sample.
        #
        #   - If the sample window is [-5, -5], the buffer will keep the 5 past samples but will also 'look'
        #     5 samples into the future. In practice, this means that the incoming EEG/EMG samples will be
        #     delayed by 5 samples. This can be useful, e.g., for various kinds of filtering.
        #
        self.sample_window = [-5, 0]

        # To work in real-time, the pipeline will precompute the capacitor voltages and pulse parameters for the targets used
        # in the experiment. For that purpose, the decider needs to provide the pipeline with a list of targets via the
        # 'targets' attribute below.
        #
        # Any targets used in the experiment should be defined here. If a target is missing from the list, the pipeline will
        # compute it on-the-fly when needed, but that introduces a delay of 10-20 seconds before the trial is performed.
        self.targets = [
            LOW_INTENSITY_TARGET,
            HIGH_INTENSITY_TARGET,
            PAIRED_PULSE_TARGET,
        ]

        # Keep track of which target type to use next. This is for alternating between low intensity, high intensity, and
        # paired-pulse targets.
        self.target_type = 0

        # As an example, write the decision times (= time of the EEG sample based on which the trial was decided to be performed)
        # into a file. Other information about the analysis can be written into a file, as well, for later analysis.
        self.file = open("decision_times.txt", "a")

        # Initialize a multiprocessing pool with one process. The pool is used so that EEG/EMG sample stream processing can be
        # continued while a long-standing computation is performed in the background.
        self.pool = multiprocessing.Pool(processes=1)

        self.model_training_interval = 10  # seconds
        self.model_training_interval_in_samples = int(self.sampling_frequency * self.model_training_interval)

        # A variable to store the ongoing training process. This is used to check if the training process is finished.
        self.training_process = None

    def process(self, current_time, timestamps, valid_samples, eeg_samples, emg_samples, current_sample_index, ready_for_trial):
        """The 'process' method is called by the pipeline for each new EEG/EMG sample. The method receives the following arguments:

           - current_time:
               The time of the current sample in seconds.

           - timestamps:
               A list of timestamps for the EEG/EMG samples; a NumPy array of shape (num_of_samples,), where num_of_samples
               is the number of samples in the buffer defined by self.sample_window (e.g, 6 if the sample window is [-5, 0]).

          - valid_samples:
               A list of booleans indicating whether the EEG/EMG samples are valid; a NumPy array of shape (num_of_samples,).
               The validity of the samples is determined on a sample-by-sample basis by the preprocessor of the pipeline.

               The example preprocessor (used as the default) marks all samples as valid, except for one second after
               each pulse.

          - eeg_samples:
               A NumPy array of shape (num_of_eeg_channels, num_of_samples), where num_of_eeg_channels is the number of EEG
               channels and num_of_samples is the number of samples in the buffer defined by self.sample_window.

          - emg_samples:
               A NumPy array of shape (num_of_emg_channels, num_of_samples), where num_of_emg_channels is the number of EMG
               channels and num_of_samples is the number of samples in the buffer defined by self.sample_window.

          - current_sample_index:
               The index of the current sample in the buffer. It always points to the last sample of eeg_samples and
               emg_samples buffers if self.sample_window is set to [-n, 0]. However, if self.sample_window is set,
               e.g., to [-10, 5], current_sample_index would be 10 while the buffers would contain 16 samples.

               Note that this is redundant information, as the current sample can be calculated from self.sample_window,
               but it is provided for convenience.

          - ready_for_trial:
               A boolean indicating whether the pipeline is ready to perform a trial. The pipeline is not ready for a trial
               if the previous trial is still ongoing or if the pipeline is in the process of precomputing the targets.

               For instance, if the pipeline has just started, it will not be ready for a trial until all the targets are
               precomputed, which may take 10-20 seconds for each target.

               Another example is when the decider has decided to perform a trial, say, 0.5 seconds after the current sample.
               During these 0.5 seconds, the pipeline is not ready for another trial, and ready_for_trial will be False.

               The decider should not perform a trial if the pipeline is not ready. However, it can do other processing, such
               as logging, filtering, computing metrics, or training models. If the decider still decides to perform a trial,
               the pipeline will print a warning and ignore the trial."""

        # Increment the sample count for each new sample.
        self.sample_count += 1

        # As an example, print the value of the first EEG channel at the current sample index into the decider log. Note that
        # using 'print' function here would congest the log with too many messages. Instead, use 'print_throttle' to print
        # the message once every second.
        #
        # Beware of using 'print_throttle' in several places in the code, as the throttling interval is shared among all calls.
        print_throttle(eeg_samples[1, current_sample_index])

        # Silently return without performing a trial if the pipeline is not ready for a trial. Usually it is good to be verbose
        # and log the reason for not performing a trial, but not being ready for a trial is a common situation that does not
        # require logging.
        if not ready_for_trial:
            return

        # Similarly, do not perform a trial if there are invalid samples in the current buffer. Assuming that the stimulation
        # decision is based on analysis of the whole buffer, it is better to conservatively skip the trial if there are any
        # invalid samples.
        if not np.all(valid_samples):
            return

        # Start training a model every 10 seconds, as defined by self.model_training_interval_in_samples.
        #
        # See Python's multiprocessing documentation for more information on how to use the multiprocessing module:
        # https://docs.python.org/3/library/multiprocessing.html
        #
        if self.sample_count % self.model_training_interval_in_samples == 0:
            print("Starting to train a model at time {:.4f}".format(current_time))
            self.training_process = self.pool.apply_async(train_model, (
                eeg_samples,
            ))

        # If the training is finished, get the model parameters and execution time.
        if self.training_process is not None:
            try:
                model_parameters, execution_time = self.training_process.get(timeout=0)

                print("Finished training a model at time {:.4f}".format(current_time))
                self.training_process = None

                # Print the latest model parameters and execution time.
                print("Model parameters: {}, Execution time (s): {:.4f}".format(
                    model_parameters,
                    execution_time,
                    current_time))

            except multiprocessing.TimeoutError as e:
                pass

        # Otherwise, perform trial once every two seconds, as defined by self.intertrial_interval_in_samples.
        if self.sample_count % self.intertrial_interval_in_samples != 0:
            return

        # Write the decision time into a file.
        self.file.write("{:.4f}\n".format(current_time))

        # Flush the file after each write so that the data is written to disk immediately.
        self.file.flush()

        # Cycle among the three target types.
        targets = self.targets[self.target_type]
        self.target_type = (self.target_type + 1) % len(self.targets)

        # Compute the pulse time based on the current time and the minimum delay before the pulse.
        start_time = current_time + MINIMUM_DELAY_BEFORE_PULSE

        # If performing the paired-pulse target, use a delay of 100 ms between the pulses.
        if len(targets) == 2:
            pulse_times = [start_time, start_time + 0.1]
        else:
            pulse_times = [start_time]

        # Some additional logging for debugging purposes.
        print("Decided at time {:.4f} to perform trial at time {:.4f} with {} target(s)".format(
            current_time,
            start_time,
            len(targets)))

        # A trial consists of the targets, the pulse times, and the information for trigger outputs of the mTMS device.
        trial = {
            'targets': targets,
            'pulse_times': pulse_times,
            'triggers': TRIGGERS,
        }

        return {
            'trial': trial,

            # In addition to performing a trial using the mTMS device, the decider can create a trigger signal using LabJack T4.
            # This is useful for triggering commercial TMS devices or other devices that require a trigger signal, and when the
            # mTMS device is not available. For now, timing the LabJack trigger into the future is not supported, so the trigger
            # is always timed as soon as possible.
            'trigger_labjack': False,
        }


def train_model(X):
    # Train a model here. For now, just sleep for a while.
    time.sleep(6.0)

    # Return a dummy result; the first element are the model parameters, and the second element is the execution time in seconds.
    return np.array([1.0, 2.0, 3.0]), 0.2
