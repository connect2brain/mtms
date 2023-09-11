from ..base.base_pipeline_stage import BasePipelineStage
from ..base.events import Pulse
from ..base.execution_condition import ExecutionCondition
from ..base.utils import analyze_eeg, get_default_waveform


# XXX: Bitrotten as of July 2023, needs to be updated and cleaned up.

class PipelineStage(BasePipelineStage):
    def __init__(self):
        super().__init__(auto_enqueue=True, window_size=5000)
        self.event_index = 1

    def init_session(self):
        super().init_session()

        return []

    def end_session(self):
        super().end_session()

        return []

    def data_received(self, sample, time, first_sample_of_session):
        super().data_received(sample, time, first_sample_of_session)

        is_spiking = analyze_eeg(self.samples.get_buffer())

        # if not spiking at the moment, do not do anything
        if not is_spiking:
            return []

        waveform = get_default_waveform(channel=1)

        event_info = {
            "id": self.event_index,
            "execution_condition": ExecutionCondition.IMMEDIATE.value,
            "execution_time": time,
        }
        self.event_index += 1

        pulse = Pulse(channel=1, waveform=waveform, event_info=event_info)

        return [pulse]
