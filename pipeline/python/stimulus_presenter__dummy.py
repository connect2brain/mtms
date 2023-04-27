from .base.base_python_processor import BaseProcessor
from .base.events import Sample


class Processor():
    def __init__(self):
        pass

    def init_experiment(self):
        return []

    def end_experiment(self):
        return []

    def data_received(self, execution_time, state):
        print(f"Stimulus received: {state} at {execution_time}")

        return []
