from ..base.base_pipeline_stage import BasePipelineStage
from ..base.events import Sample


class PipelineStage(BasePipelineStage):
    def __init__(self):
        super().__init__(
            disable_sample_buffer=True,
        )

    def init_session(self):
        super().init_session()
        return []

    def end_session(self):
        super().end_session()
        return []

    def data_received(self, sample, time, first_sample_of_session):
        super().data_received(sample, time, first_sample_of_session)

        sample = Sample(
            sample=sample,
            time=time,
            first_sample_of_session=first_sample_of_session
        )
        return [sample]
