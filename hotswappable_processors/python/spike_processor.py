from .base_python_processor import BaseProcessor
from .events import Pulse
from .execution_condition import ExecutionCondition
from .utils import analyze_eeg, get_default_waveform


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

        is_spiking = analyze_eeg(self.samples.get_buffer())

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
