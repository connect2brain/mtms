from .base.base_python_processor import BaseProcessor
from .base.events import TriggerOut, Stimulus
from .base.execution_condition import ExecutionCondition


class Processor(BaseProcessor):
    def __init__(self):
        super().__init__(auto_enqueue=True, window_size=5000, channels=62)
        self.event_index = 1
        self.samples_collected = 0
        self.sampling_frequency = 5000
        self.samples_needed = 100

    def init_experiment(self):
        super().init_experiment()

        return []

    def end_experiment(self):
        super().end_experiment()

        return []

    def data_received(self, sample, time, first_sample_of_experiment):
        super().data_received(sample, time, first_sample_of_experiment)
        self.samples_collected += 1

        if self.samples.full and self.samples_collected % self.samples_needed == 0:
            samples = self.samples.get_buffer()

            event_info = {
                "id": self.event_index,
                "execution_condition": ExecutionCondition.IMMEDIATE.value,
                "execution_time": time,
            }
            self.event_index += 1

            #event = TriggerOut(1, 1000, event_info)
            event = Stimulus(1, event_info)

            self.samples_collected = 0

            return [event]

        return []
