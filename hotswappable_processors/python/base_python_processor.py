from abc import abstractmethod

import numpy as np
from enum import Enum


class Pulse:
    def __init__(self, channel, pieces, event, event_type) -> None:
        self.channel = channel
        self.pieces = pieces
        self.event = event
        self.event_type = event_type


class Charge:
    def __init__(self, channel, target_voltage, event, event_type) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event = event
        self.event_type = event_type


class Discharge:
    def __init__(self, channel, target_voltage, event, event_type) -> None:
        self.channel = channel
        self.target_voltage = target_voltage
        self.event = event
        self.event_type = event_type


class EventType(Enum):
    PULSE = 0
    CHARGE = 1
    DISCHARGE = 2


event_type = "pulse"
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
event = {
    "id": 69,
    "execution_condition": 3,
    "time_us": 10392930
}

pulse = Pulse(3, pieces, event, EventType.PULSE.value)
charge = Charge(3, 1200, event, EventType.CHARGE.value)
discharge = Discharge(3, 20, event, EventType.DISCHARGE.value)

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
