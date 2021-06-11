import inspect
import os
import time
import queue
from threading import Thread
from typing import Any, Callable, Dict, List, Optional, Union

Event = str

# TODO: Be more stringent and explicit about data types that can be sent via SocketIO.
SocketIOData = Union[str, dict]

EventHandler = Callable[[SocketIOData], None]

ClientId = str

class MockSocketIO:
    """A class for mocking SocketIO class.

    """

    def __init__(self, broadcasted: List[SocketIOData] = [], sent_to_specific_client: List[SocketIOData] = []) -> None:
        """Initialize the mock object.

        Parameters
        ----------
        broadcasted
            A list for storing data broadcasted by SocketIO. If not given, the
            data is stored into a dummy list, not accessible from outside.
        sent_to_specific_client
            A list for storing data sent to the particular client who creates the object. If not given, the
            data is stored into a dummy list, not accessible from outside.
        """
        self.event_handlers: Dict[Event, EventHandler] = {}

        self.broadcasted: List[SocketIOData] = broadcasted
        self.sent_to_specific_client: List[SocketIOData] = sent_to_specific_client

        self.client_id: ClientId = 'test_client'
        self.environment: Dict[str, Any] = {}

    def on(self, event: Event, handler: EventHandler) -> None:
        """Register a new event handler. Mocks 'on' function in AsyncServer.

        """
        self.event_handlers[event] = handler

    async def emit(self, event: Event, data: SocketIOData, to: ClientId = None) -> None:
        """Emit an event with the accompanying data. Mocks 'emit' function in SocketIO.

        """
        if to is None:
            self.broadcasted.append({
                'event': event,
                'data': data,
            })
        else:
            self.sent_to_specific_client.append({
                'event': event,
                'data': data,
            })

    async def simulate_event(self, event: Event, data: Optional[SocketIOData] = None) -> None:
        """Simulate a client sending an event. Support both special events, such as 'connect',
        and events with and without attached data, such as the event 'command' with the data 'stimulate'.

        """
        if event in self.event_handlers.keys():
            handler = self.event_handlers[event]

            # 'Connect' event is a special case: when connecting, the environment is sent as a
            # keyword argument to the handler of the new connection.
            if event == 'connect':
                if inspect.iscoroutinefunction(handler):
                    await handler(
                        client_id=self.client_id,
                        environment=self.environment,
                    )
                else:
                    handler(
                        client_id=self.client_id,
                        environment=self.environment,
                    )
            else:
                if inspect.iscoroutinefunction(handler):
                    await handler(
                        client_id=self.client_id,
                        data=data,
                    )
                else:
                    handler(
                        client_id=self.client_id,
                        data=data,
                    )
