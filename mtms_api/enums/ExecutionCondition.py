from enum import Enum

class ExecutionCondition(Enum):
    TIMED = 0
    TRIGGERED = 1
    INSTANT = 2
