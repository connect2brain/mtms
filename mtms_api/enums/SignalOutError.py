from .Error import Error

class SignalOutError(Error):
    NO_ERROR = 0
    INVALID_EXECUTION_CONDITION = 1
    LATE = 2
    SIGNALOUT_FAILURE = 3
    UNKNOWN_ERROR = 4
