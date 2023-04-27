from enum import Enum


class ExecutionCondition(Enum):
    TIMED = 0
    WAIT_FOR_TRIGGER = 1
    IMMEDIATE = 2
