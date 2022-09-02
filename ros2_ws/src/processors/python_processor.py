class StimulationEvent:
    def __init__(self, channel, pieces, event_info) -> None:
        self.channel = channel
        self.pieces = pieces
        self.event_info = event_info

class Processor:
    def __init__(self) -> None:
        self.data = []

        self.counter = 0

    def init(self):
        print("in python init")

    def data_received(self, data, time_us, first_sample_of_experiment):
        self.counter += 1
        print(f"counter: {self.counter}, data: {data}, time_us: {time_us}, first sample of experiment: {first_sample_of_experiment}")

        channel = 1
        pieces = [
                {
                    "mode": 1,
                    "duration_in_ticks": 500
                },
                {
                    "mode": 1,
                    "duration_in_ticks": 500
                },
                {
                    "mode": 1,
                    "duration_in_ticks": 500
                }
            ]
        event_info = {
                "event_id": 1,
                "execution_condition": 3,
                "time_us": 10392930
            }

        event = StimulationEvent(channel, pieces, event_info)

        return [event]