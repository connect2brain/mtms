import numpy as np

import cpp_bindings


# Override Python's native print() function.
def print(x):
    cpp_bindings.log(str(x))


class Presenter:
    def __init__(self):
        pass

    def process(self, state, parameter, duration):
        return True
