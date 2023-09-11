from enum import Enum


class EventType(Enum):
    PULSE = 0
    CHARGE = 1
    DISCHARGE = 2
    TRIGGER_OUT = 3
    STIMULUS = 4


class Stimulus:
    def __init__(self, state, event_info) -> None:
        self.state = state
        self.event_info = event_info
        self.event_type = EventType.STIMULUS.value


class Pulse:
    def __init__(self, channel, waveform, event_info) -> None:
        self.channel = channel
        self.waveform = waveform
        self.event_info = event_info
        self.event_type = EventType.PULSE.value


class Charge:
    def __init__(self, channel, target_voltage, event_info) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event_info = event_info
        self.event_type = EventType.CHARGE.value


class Discharge:
    def __init__(self, channel, target_voltage, event_info) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event_info = event_info
        self.event_type = EventType.DISCHARGE.value


class TriggerOut:
    def __init__(self, port, duration_us, event_info) -> None:
        self.port = port
        self.duration_us = duration_us
        self.event_info = event_info
        self.event_type = EventType.TRIGGER_OUT.value


class Sample:
    def __init__(self, sample, time, first_sample_of_session):
        self.sample = sample
        self.time = time
        self.first_sample_of_session = first_sample_of_session
