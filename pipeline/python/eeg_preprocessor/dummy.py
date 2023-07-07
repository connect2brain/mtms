from ..base.base_pipeline_stage import BasePipelineStage
from ..base.events import Sample


class PipelineStage(BasePipelineStage):
    def __init__(self):
        super().__init__(
            disable_sample_buffer=True,
            channels=62,
        )

    def init_experiment(self):
        super().init_experiment()
        return []

    def end_experiment(self):
        super().end_experiment()
        return []

    def data_received(self, sample, time, first_sample_of_experiment):
        super().data_received(sample, time, first_sample_of_experiment)

        sample = Sample(
            sample=sample,
            time=time,
            first_sample_of_experiment=first_sample_of_experiment
        )
        return [sample]
