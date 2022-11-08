from .base_python_processor import BaseProcessor, Charge, Discharge, Pulse
from .thresholding import RealtimePeakDetection


class Processor(BaseProcessor):
    def __init__(self):
        self.eeg_data_index = 0
        self.data = []
        self.file = None
        self.peak_at = 0
        self.peak_detection = RealtimePeakDetection([0] * 50, 30, 5.5, 0.7)
        self.peak_over = True
        self.peaks_detected = 0

    def init_experiment(self):
        print("in python init experiment")
        self.file = open('./data/eeg_python.csv', 'w')
        self.file.write('c3,filtered,peak\n')

    def end_experiment(self):
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

    def data_received(self, data, time, first_sample_of_experiment):
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
        # if self.file:
        #    self.file.write(f"{c3},{filtered},{peak_mark}\n")

        # if send_pulse:
        #    return [pulse]

        return [charge]
