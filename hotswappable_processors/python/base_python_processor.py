from abc import abstractmethod


class BaseProcessor:
    @abstractmethod
    def init_experiment(self):
        pass

    @abstractmethod
    def end_experiment(self):
        pass

    @abstractmethod
    def data_received(self, data, time, first_sample_of_experiment):
        pass
