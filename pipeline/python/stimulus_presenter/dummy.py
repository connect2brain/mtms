from ..base.base_pipeline_stage import BasePipelineStage
from ..base.events import Sample


class PipelineStage():
    def __init__(self):
        pass

    def init_session(self):
        return []

    def end_session(self):
        return []

    def data_received(self, execution_time, state):
        print(f"Stimulus received: {state} at {execution_time}")

        return []
