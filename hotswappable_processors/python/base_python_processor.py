from abc import abstractmethod
from .buffer import Buffer


class BaseProcessor:

    def __init__(self, auto_enqueue=False, window_size=5000):
        self.auto_enqueue = auto_enqueue
        self.window_size = window_size
        self.samples = Buffer(window_size=5000)

    @abstractmethod
    def init_experiment(self):
        pass

    @abstractmethod
    def end_experiment(self):
        pass

    def enqueue(self, sample):
        self.samples.append(sample)

    @abstractmethod
    def data_received(self, sample, time, first_sample_of_experiment):
        if self.auto_enqueue:
            self.enqueue(sample)

        pass
