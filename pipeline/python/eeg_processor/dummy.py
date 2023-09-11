from ..base.base_pipeline_stage import BasePipelineStage
from ..base.events import TriggerOut, Stimulus
from ..base.execution_condition import ExecutionCondition


class PipelineStage(BasePipelineStage):
    # Corresponds to 1 s of data with sampling frequency 1000 Hz.
    SAMPLE_BUFFER_SIZE = 1000

    def __init__(self):
        super().__init__(
            sample_buffer_size=self.SAMPLE_BUFFER_SIZE,
            analysis_interval_in_samples=100,
        )
        self.event_index = 0

    def init_session(self):
        super().init_session()
        return []

    def end_session(self):
        super().end_session()
        return []

    def data_received(self, sample, time, first_sample_of_session):
        super().data_received(sample, time, first_sample_of_session)

        if not self.is_ready_to_analyze():
            return []

        samples = self.sample_buffer.get_buffer()

        event = self.create_trigger_out_event(time)
        return [event]

    def create_stimulus_event(self, time):
        event_info = {
            "id": self.event_index,
            "execution_condition": ExecutionCondition.IMMEDIATE.value,
            "execution_time": time,
            "decision_time": time,
        }
        event = Stimulus(
            state=1,
            event_info=event_info,
        )

        self.event_index += 1

        return event

    # Unused in this example script.
    def create_trigger_out_event(self, time):
        event_info = {
            "id": self.event_index,
            "execution_condition": ExecutionCondition.IMMEDIATE.value,
            "execution_time": time,
            "decision_time": time,
        }
        event = TriggerOut(
            port=1,
            duration_us=1000,
            event_info=event_info,
        )

        self.event_index += 1

        return event
