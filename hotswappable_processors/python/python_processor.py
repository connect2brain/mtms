from .base_python_processor import BaseProcessor, ChargeEvent, EventType, StimulationEvent


class Processor(BaseProcessor):
    def __init__(self) -> None:
        self.eeg_data_index = 0
        # self.data = np.array

    def init_experiment(self):
        print("PYTHON: initialized experiment")
        return []

    def end_experiment(self):
        print("PYTHON: ended experiment")
        return []

    def data_received(self, data, time_us, first_sample_of_experiment):
        self.eeg_data_index += 1
        event_info = {
            "event_id": 69,
            "execution_condition": 3,
            "time_us": 100 * self.eeg_data_index
        }
        pieces = [
            {
                "mode": 1,
                "duration_in_ticks": 500
            },
            {
                "mode": 2,
                "duration_in_ticks": 500
            },
            {
                "mode": 3,
                "duration_in_ticks": 500
            }
        ]
        # charge_event = ChargeEvent(3, 1200, event_info, EventType.CHARGE_EVENT.value)
        # return [charge_event, charge_event, charge_event]
        pulse_event = StimulationEvent(3, pieces, event_info, EventType.STIMULATION_PULSE_EVENT.value)
        return [pulse_event]
