import time
from abc import abstractmethod

from .circular_buffer import CircularBuffer


class BasePipelineStage:
    def __init__(self, disable_sample_buffer=False, sample_buffer_size=5000, analysis_interval_in_samples=1):
        self._disable_sample_buffer = disable_sample_buffer
        self._sample_buffer_size = sample_buffer_size
        self._analysis_interval_in_samples = analysis_interval_in_samples

        self.num_of_samples = 0
        self._num_of_samples_since_last_analysis = 0

        self.sample_buffer = None
        self.num_of_channels = None

    @abstractmethod
    def init_experiment(self):
        pass

    @abstractmethod
    def end_experiment(self):
        pass

    @abstractmethod
    def data_received(self, sample, time, first_sample_of_experiment):
        # Update number of samples received.
        self.num_of_samples += 1

        sample_buffer_full = self.sample_buffer is not None and self.sample_buffer.is_full

        if self._disable_sample_buffer or sample_buffer_full:
            self._num_of_samples_since_last_analysis += 1

        self._num_of_samples_since_last_analysis = self._num_of_samples_since_last_analysis % self._analysis_interval_in_samples

        # Return if sample buffer is disabled.
        if self._disable_sample_buffer:
            return

        # Initialize sample buffer if uninitialized.
        if self.sample_buffer is None:
            self.num_of_channels = len(sample)

            self.sample_buffer = CircularBuffer(
                window_size=self._sample_buffer_size,
                columns=self.num_of_channels,
            )

        # Append to the sample buffer.
        self.sample_buffer.append(sample)

    def is_ready_to_analyze(self):
        if not self.sample_buffer.is_full:
            return False

        return self._num_of_samples_since_last_analysis % self._analysis_interval_in_samples == 0

    def time_func(self, func):
        start_time = time.time()
        result = func()
        end_time = time.time()

        elapsed_time = end_time - start_time
        print('Time elapsed: {:.1f} milliseconds'.format(1000 * elapsed_time))

        return result
