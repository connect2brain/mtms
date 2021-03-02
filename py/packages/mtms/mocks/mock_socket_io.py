import os
import time
import queue
from threading import Thread

class MockSocketIO:
    """A class for mocking SocketIO class.

    """

    def __init__(self, broadcasted=[]):
        """Initialize the mock object.

        Parameters
        ----------
        broadcasted : list
            A list for storing data broadcasted by SocketIO. If not given, the data is stored into a dummy list,
            not accessible from outside.
        """
        self.event_handlers = {}
        self.broadcasted = broadcasted

    def on_event(self, event, handler):
        """Register a new event handler. Mocks 'on_event' function in SocketIO.

        """
        self.event_handlers[event] = handler

    def emit(self, event, data):
        """Emit an event with the accompanying data. Mocks 'emit' function in SocketIO.

        XXX: SocketIO object's 'emit' function broadcasts the data to all clients, therefore the data is appended
          to a list named 'broadcasted'.
        """
        self.broadcasted.append({
            'event': event,
            'data': data,
        })

    def simulate_event(self, event=None, data=None):
        """Simulate a client sending an event.

        """
        if event in self.event_handlers.keys():
            if data is None:
                self.event_handlers[event]()
            else:
                self.event_handlers[event](data)
