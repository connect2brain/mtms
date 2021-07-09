#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import json
from functools import partial
from typing import Any, Dict, List, Optional, Tuple, TypedDict

from socketio import AsyncServer

from mtms.util.util import drop_unique
from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

Position = Optional[Tuple[float, float, float]]

class AddPointData(TypedDict):
    position: Position

class RemovePointData(TypedDict):
    name: str

class PointAddedData(TypedDict):
    visible: bool
    name: str
    type: str
    comment: str
    position: Position

class NeuroNavigationMessage(TypedDict):
    topic: str
    data: Any

class PlannerServer:
    """A backend server for the planner.

    """

    _SOCKETIO_UPDATE_POINTS: str = 'planner.points'
    _SOCKETIO_UPDATE_POSITION: str = 'planner.position'
    _SOCKETIO_UPDATE_COIL_AT_TARGET: str = 'planner.coil_at_target'
    _SOCKETIO_NEW_CLIENT: str = 'planner.new_client'

    _COMMAND_ADD_POINT: str = 'point.add'
    _COMMAND_REMOVE_POINT: str = 'point.remove'

    def __init__(self, kafka: Kafka, socketio: AsyncServer) -> None:
        """Initialize the planner server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            An AsyncServer object to which the event listeners are added.
        """
        self._socketio: AsyncServer = socketio
        self._kafka: Kafka = kafka

        self._position: Position = None
        self._neuronavigation_project_open: bool = False
        self._coil_at_target: bool = False
        self._added_points_total: int = 0

        # Socket.IO event handlers

        socketio.on(
            event='from_neuronavigation',
            handler=self._handle_neuronavigation_message,
        )

        # A handler for adding a new point in the front-end.
        socketio.on(
            event=self._COMMAND_ADD_POINT,
            handler=self._handle_add_point,
        )

        # A handler for removing a point in the front-end.
        socketio.on(
            event=self._COMMAND_REMOVE_POINT,
            handler=self._handle_remove_point,
        )

        # A handler for a new front-end client.
        socketio.on(
            event=self._SOCKETIO_NEW_CLIENT,
            handler=self._handle_new_client,
        )

        self._points: List[PointAddedData] = []

        self._setup_background_tasks()

    def _setup_background_tasks(self):
        """Setup the background tasks, namely, a task for listening for a Kafka command
        to add a new point.

        """
        self.background_tasks: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=self._COMMAND_ADD_POINT,
                callback=self._point_added,
            ),
            KafkaListener(
                kafka=self._kafka,
                topic=self._COMMAND_REMOVE_POINT,
                callback=self._point_removed,
            ),
        ]

    async def _handle_neuronavigation_message(self, client_id: str, msg: NeuroNavigationMessage) -> None:
        """A handler for messages from neuronavigation.

        Broadcasts the message received from neuronavigation.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        msg
            The message received from neuronavigation, see type NeuroNavigationMessage for the structure.
        """
        topic: str = msg['topic']
        data: Any = msg['data']
        logging.info("Received a message from neuronavigation in topic '{}'".format(topic))

        # Triggered when the cross is moved around in neuronavigation. React by updating the new cross
        # position to the frontend.
        if topic == "Set cross focal point":
            self._position = data['position'][0:3]
            await self._update_position()

        # Triggered when a project is opened in neuronavigation. React by sending the current points to
        # neuronavigation so that they can be shown in the project.
        elif topic == "Enable state project":
            self._neuronavigation_project_open = data['state']
            if self._neuronavigation_project_open:
                await self._update_neuronavigation()

        # Triggered when the neuronavigation evaluates if the coil is at the target: accordingly, the state
        # is either True or False.
        elif topic == "Coil at target":
            self._coil_at_target = data['state']
            await self._update_coil_at_target()

    def _handle_add_point(self, client_id: str, data: AddPointData) -> None:
        """When a command is received from the front-end to add a new point, create the
        initial values for the point and pass the message to create a point to Kafka.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end with the command. Should consist of
            'position' key, with a value that consists of a list of three floats.

            See type AddPointData for the specification.

            An example:

            {
                'position': [1.0, 2.0, 3.0],
            }
        """
        logging.info("Received a command from the front-end to add a new point")

        self._added_points_total += 1
        value: PointAddedData = {
            'visible': False,
            'name': "Target-{}".format(self._added_points_total),
            'type': "Target",
            'comment': "",
            'position': data['position'],
        }

        self._kafka.produce(
            topic=self._COMMAND_ADD_POINT,
            value=bytes(json.dumps(value), encoding='utf8')
        )

    def _handle_remove_point(self, client_id: str, data: RemovePointData) -> None:
        """When a command is received from the front-end to remove a point, pass the
        message to remove a point to Kafka.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The data sent from the front-end with the command.
            See type RemovePointData for the specification.

            An example:

            {
                'name': "Target-1",
            }
        """
        logging.info("Received a command from the front-end to remove a point")

        self._kafka.produce(
            topic=self._COMMAND_REMOVE_POINT,
            value=bytes(json.dumps(data), encoding='utf8')
        )

    async def _handle_new_client(self, client_id: str, _) -> None:
        """When a new client connects, send all needed data to the client, i.e., the points
        and the position.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        """
        logging.info("A new client connected with the id {}".format(client_id))

        await self._update_points(
            client_id=client_id,
        )
        await self._update_position(
            client_id=client_id,
        )
        await self._update_coil_at_target(
            client_id=client_id,
        )

    async def _update_points(self, client_id: str = None) -> None:
        """Update planner points to the frontend.

        Parameters
        ----------
        client_id
            The client id. If provided, the points are sent only to the client with that id.
            If not provided (the default), the points are broadcast to all clients.
        """
        await self._socketio.emit(
            event=self._SOCKETIO_UPDATE_POINTS,
            data=self._points,
            client_id=client_id,
        )

    async def _update_position(self, client_id: str = None) -> None:
        """Update the position, if defined, to the frontend.

        Parameters
        ----------
        client_id
            The client id. If provided, the position is sent only to the client with that id.
            If not provided (the default), the position is broadcast to all clients.
        """
        if self._position is not None:
            await self._socketio.emit(
                event=self._SOCKETIO_UPDATE_POSITION,
                data=self._position,
                client_id=client_id,
            )

    async def _update_coil_at_target(self, client_id: str = None) -> None:
        """Update the indicator for coil being at the target.

        Parameters
        ----------
        client_id
            The client id. If provided, the position is sent only to the client with that id.
            If not provided (the default), the position is broadcast to all clients.
        """
        await self._socketio.emit(
            event=self._SOCKETIO_UPDATE_COIL_AT_TARGET,
            data=self._coil_at_target,
            client_id=client_id,
        )

    async def _send_to_neuronavigation(self, topic: str, data: Any) -> None:
        """Given a topic and data of any type, send a message to neuronavigation in that topic
        and passing on the given data.

        Parameters
        ----------
        topic
            The topic for the internal communication that the neuronavigation software does. E.g.,
            "Add marker".
        data
            Any data that will be sent in the given topic.

        """
        await self._socketio.emit(
            event="to_neuronavigation",
            data={
                "topic": topic,
                "data": data,
            }
        )

    async def _update_neuronavigation(self) -> None:
        """Send all points to neuronavigation to display them.

        """
        markers = []

        for id, point in enumerate(self._points):
            coord: Position = point['position']
            marker_data = {
                'ball_id': id,
                'coord': coord,
                'size': 2,
                'colour': (1.0, 1.0, 0.0),
            }
            markers.append(marker_data)

        await self._send_to_neuronavigation(
            topic="Set markers",
            data={
                "markers": markers,
            }
        )

    async def _point_added(self, topic: str, data: str) -> None:
        """Handle the command received from Kafka to add a new point, specifically, pass the
        command on to the front-end and neuronavigation.

        Parameters
        ----------
        topic
            The name of the topic in which the command is sent.
        data
            A json dict consisting of the attributes of the new point. The dict should
            consist of the keys 'visible', 'name', 'type', 'comment', and 'position'.

            An example:

            {
                'visible': False,
                'name': "Target-1",
                'type': "Target",
                'comment': "This is a comment.",
                'position': [1.0, 2.0, 3.0],
            }
        """
        # TODO: The format and the content of the received message needs to be checked.
        #       A natural place for those checks would be inside Kafka listener, so this
        #       function can then assume a valid message.
        data: PointAddedData = json.loads(data)

        # Update backend
        self._points.append(data)

        # Update frontend
        await self._update_points()

        # Update neuronavigation
        await self._update_neuronavigation()

    async def _point_removed(self, topic: str, data: str) -> None:
        """Handle the command received from Kafka to remove a point.

        Parameters
        ----------
        topic
            The name of the topic in which the command is sent.
        data
            A json dict consisting of the attributes of the new point. The dict should
            consist of the key 'name'.

            An example:

            {
                'name': "Target-1",
            }
        """
        data: RemovePointData = json.loads(data)

        # Update backend
        index: int = drop_unique(self._points, lambda point: point['name'] == data['name'])

        # Update frontend
        await self._update_points()

        # Update neuronavigation
        await self._update_neuronavigation()
