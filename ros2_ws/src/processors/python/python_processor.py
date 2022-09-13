import numpy as np


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

SAMPLING_RATE = 5000


class real_time_peak_detection():
    def __init__(self, array, lag, threshold, influence):
        self.y = list(array)
        self.length = len(self.y)
        self.lag = lag
        self.threshold = threshold
        self.influence = influence

        self.signals = [0] * len(self.y)
        self.filteredY = np.array(self.y).tolist()
        self.avgFilter = [0] * len(self.y)
        self.stdFilter = [0] * len(self.y)
        self.avgFilter[self.lag - 1] = np.mean(self.y[0:self.lag]).tolist()
        self.stdFilter[self.lag - 1] = np.std(self.y[0:self.lag]).tolist()

    def thresholding_algo(self, new_value):
        i = len(self.y) - 1
        self.y.append(new_value)

        self.signals += [0]
        self.filteredY += [0]
        self.avgFilter += [0]
        self.stdFilter += [0]

        if len(self.y) > self.length:
            self.y.pop(0)
        if len(self.signals) > self.length:
            self.signals.pop(0)
        if len(self.filteredY) > self.length:
            self.filteredY.pop(0)
        if len(self.avgFilter) > self.length:
            self.avgFilter.pop(0)
        if len(self.stdFilter) > self.length:
            self.stdFilter.pop(0)

        if abs(self.y[i] - self.avgFilter[i - 1]) > (self.threshold * self.stdFilter[i - 1]):

            if self.y[i] > self.avgFilter[i - 1]:
                self.signals[i] = 1
            else:
                self.signals[i] = -1

            self.filteredY[i] = self.influence * self.y[i] + (1 - self.influence) * self.filteredY[i - 1]
            self.avgFilter[i] = np.mean(self.filteredY[(i - self.lag):i])
            self.stdFilter[i] = np.std(self.filteredY[(i - self.lag):i])
        else:
            self.signals[i] = 0
            self.filteredY[i] = self.y[i]
            self.avgFilter[i] = np.mean(self.filteredY[(i - self.lag):i])
            self.stdFilter[i] = np.std(self.filteredY[(i - self.lag):i])

        return self.signals[i]


class Processor:
    def __init__(self) -> None:
        self.eeg_data_index = 0
        self.data = []
        self.file = None
        self.peak_at = 0
        self.peak_detection = real_time_peak_detection([0] * 50, 30, 5.5, 0.7)
        self.peak_over = True
        self.peaks_detected = 0

    def init(self):
        print("in python init")
        self.file = open('./data/eeg_python.csv', 'w')
        self.file.write('c3,filtered,peak\n')

    def close(self):
        if self.file:
            self.file.close()
            print("Closed file")
        print(f"Peaks detected: {self.peaks_detected}")

    def laplacian(self, c3, others):
        neg = [(-1) * x for x in others]
        return 4 * c3 + sum(neg)

    def average(self, c3, others):
        combined = [c3] + others
        return sum(combined) / len(combined)

    def data_received(self, data, time_us, first_sample_of_experiment):
        self.eeg_data_index += 1

        c3 = data[4]
        others = [data[20], data[22], data[24], data[26]]
        filtered = self.average(c3, others)

        self.data.append(filtered)
        if len(self.data) > 20:
            self.data.pop(0)

        filtered = sum(self.data) / len(self.data)

        signal = self.peak_detection.thresholding_algo(c3)
        if signal == 0 and not self.peak_over:
            self.peak_over = True

        peak = signal != 0

        peak_mark = 'f'
        send_pulse = False
        if peak and self.peak_over:
            self.peak_over = False
            self.peak_at = self.eeg_data_index
            peak_mark = 't'
            send_pulse = True
            self.peaks_detected += 1

        # peak_mark = 't' if peak else 'f'
        #if self.file:
        #    self.file.write(f"{c3},{filtered},{peak_mark}\n")

        #if send_pulse:
        #    return [pulse_event]

        return [pulse_event, charge_event]
