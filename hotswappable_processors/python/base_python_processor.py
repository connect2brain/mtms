from abc import abstractmethod
from circular_buffer import CircularBuffer


class BaseProcessor:

    def __init__(self, auto_enqueue=False, window_size=5000):
        self.auto_enqueue = auto_enqueue
        self.window_size = window_size
        self.samples = CircularBuffer(self.window_size)

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
