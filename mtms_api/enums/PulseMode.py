from enum import Enum

class PulseMode(Enum):
    NON_CONDUCTIVE = 0
    RISING = 1
    HOLD = 2
    FALLING = 3
    ALTERNATIVE_HOLD = 4
