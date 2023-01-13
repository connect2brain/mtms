from enum import Enum

from .util.bcolors import bcolors

class ExperimentState(Enum):
    def __new__(cls, value, color):
        entry = object.__new__(cls)
        entry._value_ = value
        entry.color = color
        return entry

    def __str__(self):
        s = self.name.lower().replace('_', ' ').capitalize()
        return "{}{}{}".format(self.color, s, bcolors.ENDC)

    STOPPED = (0, bcolors.OKBLUE)
    STARTING = (1, bcolors.OKBLUE)
    STARTED = (2, bcolors.OKGREEN)
    STOPPING = (3, bcolors.WARNING)
