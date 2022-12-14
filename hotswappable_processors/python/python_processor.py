from .base_python_processor import BaseProcessor
from .events import Charge
from .execution_condition import ExecutionCondition


class Processor(BaseProcessor):
    def __init__(self):
        super().__init__(auto_enqueue=True, window_size=5000)
        self.event_index = 1

    def init_experiment(self):
        super().init_experiment()

        return []

    def end_experiment(self):
        super().end_experiment()

        return []

    def data_received(self, sample, time, first_sample_of_experiment):
        super().data_received(sample, time, first_sample_of_experiment)

        event = {
            "id": self.event_index,
            "execution_condition": ExecutionCondition.INSTANT.value,
            "time": time
        }
        self.event_index += 1

        charge = Charge(1, 1200, event)
        return [charge]
