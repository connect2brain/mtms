from .base_python_processor import BaseProcessor
from .events import Sample


class Processor():
    def __init__(self):
        pass

    def init_experiment(self):
        return []

    def end_experiment(self):
        return []

    def data_received(self, time):
        print("Stimulus received")

        return []
