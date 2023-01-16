from enum import Enum


class EventType(Enum):
    PULSE = 0
    CHARGE = 1
    DISCHARGE = 2
    SIGNAL_OUT = 3


class Pulse:
    def __init__(self, channel, waveform, event) -> None:
        self.channel = channel
        self.waveform = waveform
        self.event = event
        self.event_type = EventType.PULSE.value


class Charge:
    def __init__(self, channel, target_voltage, event) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event = event
        self.event_type = EventType.CHARGE.value


class Discharge:
    def __init__(self, channel, target_voltage, event) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event = event
        self.event_type = EventType.DISCHARGE.value


class SignalOut:
    def __init__(self, port, duration_us, event) -> None:
        self.port = port
        self.duration_us = duration_us
        self.event = event
        self.event_type = EventType.SIGNAL_OUT.value


class Sample:
    def __init__(self, sample, time, first_sample_of_experiment):
        self.sample = sample
        self.time = time
        self.first_sample_of_experiment = first_sample_of_experiment
