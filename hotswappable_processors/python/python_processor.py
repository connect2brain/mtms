from .base_python_processor import BaseProcessor, Charge, EventType, Pulse


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

    def data_received(self, data, time, first_sample_of_experiment):
        self.eeg_data_index += 1
        event = {
            "id": 69,
            "execution_condition": 3,
            "time": 100.0 * self.eeg_data_index
        }
        waveform = [
            {
                "waveform_phase": 1,
                "duration_in_ticks": 500
            },
            {
                "waveform_phase": 2,
                "duration_in_ticks": 500
            },
            {
                "waveform_phase": 3,
                "duration_in_ticks": 500
            }
        ]
        # charge = Charge(3, 1200, event, EventType.CHARGE.value)
        # return [charge, charge, charge]
        pulse = Pulse(3, waveform, event, EventType.PULSE.value)
        return [pulse]
