import os
import time
import queue
from threading import Thread
from typing import Any, Callable, Dict, List, Optional, Union

Event = str

# TODO: Be more stringent and explicit about data types that can be sent via SocketIO.
SocketIOData = Union[str, dict]

EventHandler = Callable[[SocketIOData], None]

class MockSocketIO:
    """A class for mocking SocketIO class.

    """

    def __init__(self, broadcasted: List[SocketIOData] = []) -> None:
        """Initialize the mock object.

        Parameters
        ----------
        broadcasted
            A list for storing data broadcasted by SocketIO. If not given, the
            data is stored into a dummy list, not accessible from outside.
        """
        self.event_handlers: Dict[Event, EventHandler] = {}
        self.broadcasted: List[SocketIOData] = broadcasted

    def on(self, event: Event, handler: EventHandler) -> None:
        """Register a new event handler. Mocks 'on' function in AsyncServer.

        """
        self.event_handlers[event] = handler

    def emit(self, event: Event, data: SocketIOData) -> None:
        """Emit an event with the accompanying data. Mocks 'emit' function in SocketIO.

        XXX: SocketIO object's 'emit' function broadcasts the data to all clients,
          therefore the data is appended to a list named 'broadcasted'.
        """
        self.broadcasted.append({
            'event': event,
            'data': data,
        })

    def simulate_event(self, event: Event, data: Optional[SocketIOData] = None) -> None:
        """Simulate a client sending an event. Support both self-containing events,
        such as 'connect', and events with attached data, such as the event 'command'
        with the data 'stimulate'.

        """
        if event in self.event_handlers.keys():
            if data is None:
                self.event_handlers[event]()
            else:
                self.event_handlers[event](data)
