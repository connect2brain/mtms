class StimulationEvent:
    def __init__(self, channel, pieces, event_info, event_type) -> None:
        self.channel = channel
        self.pieces = pieces
        self.event_info = event_info
        self.event_type = event_type

class ChargeEvent:
    def __init__(self, channel, target_voltage, event_info, event_type) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event_info = event_info
        self.event_type = event_type

class Processor:
    def __init__(self) -> None:
        self.data = []

        self.counter = 0

    def init(self):
        print("in python init")

    def data_received(self, data, time_us, first_sample_of_experiment):
        self.counter += 1
        #print(f"counter: {self.counter}, time_us: {time_us}, first sample of experiment: {first_sample_of_experiment}")

        #self.data.append(data)
        #if len(self.data) > sampling_rate:
        #    self.data.pop(0)

        event_type = "stimulation"
        channel = 3
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
                "event_id": 69,
                "execution_condition": 3,
                "time_us": 10392930
            }

        pulse_event = StimulationEvent(channel, pieces, event_info, event_type)
        charge_event = ChargeEvent(channel, 1200, event_info, "charge")

        return [pulse_event, charge_event, pulse_event, charge_event, pulse_event]