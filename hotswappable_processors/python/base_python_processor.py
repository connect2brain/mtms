from abc import abstractmethod

import numpy as np
from enum import Enum


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


class DischargeEvent:
    def __init__(self, channel, target_voltage, event_info, event_type) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event_info = event_info
        self.event_type = event_type


class EventType(Enum):
    STIMULATION_PULSE_EVENT = 0
    CHARGE_EVENT = 1
    DISCHARGE_EVENT = 2


event_type = "stimulation"
channel = 3
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
event_info = {
    "event_id": 69,
    "execution_condition": 3,
    "time_us": 10392930
}

pulse_event = StimulationEvent(3, pieces, event_info, EventType.STIMULATION_PULSE_EVENT.value)
charge_event = ChargeEvent(3, 1200, event_info, EventType.CHARGE_EVENT.value)
discharge_event = DischargeEvent(3, 20, event_info, EventType.DISCHARGE_EVENT.value)

SAMPLING_RATE = 5000


class BaseProcessor:
    @abstractmethod
    def init_experiment(self):
        pass

    @abstractmethod
    def end_experiment(self):
        pass

    @abstractmethod
    def data_received(self, data, time_us, first_sample_of_experiment):
        pass
