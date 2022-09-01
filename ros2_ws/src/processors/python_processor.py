class Processor:
    def __init__(self) -> None:
        self.data = []

        self.counter = 0

    def init(self):
        print("in python init")

    def data_received(self, data):
        self.counter = data
        print(f"in data received, counter: {self.counter}")
