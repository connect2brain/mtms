from .base_python_pipeline_stage import BasePipelineStage
from .events import Sample


class PipelineStage():
    def __init__(self):
        pass

    def init_experiment(self):
        return []

    def end_experiment(self):
        return []

    def data_received(self, execution_time, state):
        print(f"Stimulus received: {state} at {execution_time}")

        return []
