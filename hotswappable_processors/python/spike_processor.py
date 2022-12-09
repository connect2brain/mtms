from .base_python_processor import BaseProcessor
from .events import Pulse
from .execution_condition import ExecutionCondition
from .utils import analyze_eeg, get_default_waveform


class Processor(BaseProcessor):
    def __init__(self) -> None:
        self.event_index = 1
        self.data = []
        self.max_data_length = 5000

    def init_experiment(self):
        return []

    def end_experiment(self):
        return []

    def enqueue(self, new_data_point):
        self.data.append(new_data_point)

        if len(self.data) > self.max_data_length:
            self.data.pop(0)

    def data_received(self, data, time, first_sample_of_experiment):
        self.enqueue(data)

        is_spiking = analyze_eeg(self.data)

        # if not spiking at the moment, do not do anything
        if not is_spiking:
            return []

        waveform = get_default_waveform(channel=1)

        event = {
            "id": self.event_index,
            "execution_condition": ExecutionCondition.INSTANT.value,
            "time": time
        }
        self.event_index += 1

        pulse = Pulse(channel=1, waveform=waveform, event=event)

        return [pulse]


