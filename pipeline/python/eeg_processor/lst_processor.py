from ..base.base_pipeline_stage import BasePipelineStage
from ..base.events import Stimulus
from ..base.execution_condition import ExecutionCondition

from .analysis_utils import (
    get_alpha_estimate,
    compute_baseline_stats,
    scale_alpha_estimate_to_step,
)


class PipelineStage(BasePipelineStage):
    # TODO: discuss these parameters
    def __init__(self, eeg_channel_inds_=[8, 9, 61], window_size_=1000, channels_=62):
        # were in template
        super().__init__(auto_enqueue=True, window_size=window_size_, channels=channels_)
        self.event_index = 0
        
        self.running = False
        
        self.sampling_frequency = 500   # EEG sampling frequency
        self.samples_needed = 250       # 0.5 seconds of data, (sfreq/2)
        self.eeg_channel_inds = eeg_channel_inds_

        # added
        # states
        self.samples_collected = 0
        self.current_alpha = -1
        self.baseline_mode = True
        self.baseline_mean = 0
        self.baseline_std = 1
        self.baseline_data = []
        self.baseline_samples_collected = 0
        self.send_baseline_over_event = False
        # TODO: discuss these parameters
        
        self.baseline_samples_needed = 240  # Number of alpha estimates included in baseline
        self.alpha_epsilon = 0.05

    def set_baseline_time(self,  baseline_time_):
        self.baseline_samples_needed = int(baseline_time_ * self.sampling_frequency) / self.samples_needed
        print("Baseline samples needed: ", self.baseline_samples_needed)


    def init_experiment(self):
        super().init_experiment()
        self.running = True

        return []

    def end_experiment(self):
        self.running = False
        super().end_experiment()
        self.samples_collected = 0
        self.baseline_samples_collected = 0
        self.send_baseline_over_event = False
        self.baseline_mode = True
        return []

    def data_received(self, sample, time, first_sample_of_experiment):
        super().data_received(sample, time, first_sample_of_experiment)
        self.samples_collected += 1

        if self.samples.full and self.samples_collected % self.samples_needed == 0:
            samples = self.samples.get_buffer()

            alpha_estimate = get_alpha_estimate(
                samples, self.eeg_channel_inds, self.sampling_frequency
            )

            if self.baseline_mode is True and self.running is True:
                print("Baseline mode is on")
                self.baseline_samples_collected += 1
                # update if baseline period is not over
                self.update_baseline_data(alpha_estimate)

            scaled_alpha_estimate = scale_alpha_estimate_to_step(
               alpha_estimate, self.baseline_mean, self.baseline_std
            )

            #self.samples_collected = 0
            print("ALPHA: {} - SCALED ALPHA: {}".format(alpha_estimate, scaled_alpha_estimate))
            return self.make_event(scaled_alpha_estimate, time)

        return []

    # added
    def make_event(self, new_alpha, time):
        if self.baseline_mode is True:
            return []
        code = self.make_event_code(new_alpha)

        self.current_alpha = new_alpha
        self.event_index += 1
        event_info = {
            "id": self.event_index,
            "execution_condition": ExecutionCondition.IMMEDIATE.value,
            "execution_time": time,
            }
        return [Stimulus(code, event_info)]


    # added
    def make_event_code(self, new_alpha):
        """Make event based on mode and alpha values."""
        if self.send_baseline_over_event is True:
            self.send_baseline_over_event = False
            print("Baseline over event sent")
            return 5

        if abs(new_alpha - self.current_alpha) < self.alpha_epsilon:
            return 1

        return 3 if new_alpha > self.current_alpha else 2

    # added
    def update_baseline_data(self, alpha_estimate):
        """Update baseline data."""

        self.baseline_data.append(alpha_estimate)

        if self.baseline_samples_collected >= self.baseline_samples_needed:
            self.baseline_mode = False
            print("Baseline period is over. Number of blocks: ", len(self.baseline_data))
            # compute baseline stats
            self.baseline_mean, self.baseline_std = compute_baseline_stats(
                self.baseline_data
            )
            self.baseline_data = []
            # print baseline stats
            print("Baseline mean: ", self.baseline_mean)
            print("Baseline std: ", self.baseline_std)
            self.send_baseline_over_event = True
            return
