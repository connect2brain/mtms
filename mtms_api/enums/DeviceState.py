from enum import Enum

from .util.bcolors import bcolors

class DeviceState(Enum):
    def __new__(cls, value, color):
        entry = object.__new__(cls)
        entry._value_ = value
        entry.color = color
        return entry

    def __str__(self):
        s = self.name.lower().replace('_', ' ').capitalize()
        return "{}{}{}".format(self.color, s, bcolors.ENDC)

    NOT_OPERATIONAL = (0, '')
    STARTUP = (1, bcolors.OKBLUE)
    OPERATIONAL = (2, bcolors.OKGREEN)
    SHUTDOWN = (3, bcolors.WARNING)
