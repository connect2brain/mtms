from enum import Enum

from .util.bcolors import bcolors

class Error(Enum):
    def __str__(self):
        error_str = self.name.lower().replace('_', ' ').capitalize()
        error_str_color = bcolors.FAIL if self.value != 0 else bcolors.OKGREEN
        error_str_colored = "{}{}{}".format(error_str_color, error_str, bcolors.ENDC)

        return error_str_colored
