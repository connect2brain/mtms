from .base_python_processor import BaseProcessor
from .events import Charge


class Processor(BaseProcessor):
    def __init__(self) -> None:
        self.event_index = 1

    def init_experiment(self):
        return []

    def end_experiment(self):
        return []

    def data_received(self, data, time, first_sample_of_experiment):
        event = {
            "id": self.event_index,
            "execution_condition": 3,
            "time": time
        }
        self.event_index += 1

        charge = Charge(1, 1200, event)
        return [charge]
