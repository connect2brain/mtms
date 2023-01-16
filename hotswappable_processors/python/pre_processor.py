from .base_python_processor import BaseProcessor
from .events import Sample


class Processor(BaseProcessor):
    def __init__(self):
        super().__init__(auto_enqueue=False, window_size=1, channels=62)

    def init_experiment(self):
        super().init_experiment()

        return []

    def end_experiment(self):
        super().end_experiment()

        return []

    def data_received(self, sample, time, first_sample_of_experiment):
        super().data_received(sample, time, first_sample_of_experiment)

        sample = Sample(sample=sample, time=time,
                        first_sample_of_experiment=first_sample_of_experiment)

        return [sample]