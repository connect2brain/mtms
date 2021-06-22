#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import json
from functools import partial
from typing import Any, Dict, List, Optional, Tuple, TypedDict

from socketio import AsyncServer

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

    _SOCKETIO_UPDATE_PLANNER: str = 'planner.update'
    _SOCKETIO_UPDATE_POSITION: str = 'position.update'

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

        if topic == "Set cross focal point":
            position: Position = data['position'][0:3]
            await self._update_position(position)

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

    async def _update_planner(self) -> None:
        """Update new planner state to the frontend.

        """
        await self._socketio.emit(
            event=self._SOCKETIO_UPDATE_PLANNER,
            data=self._points,
        )

    async def _update_position(self, position: Position) -> None:
        """Update the position to the frontend.

        """
        self._position = position

        await self._socketio.emit(
            event=self._SOCKETIO_UPDATE_POSITION,
            data=self._position,
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
        await self._update_planner()

        # Update neuronavigation
        id: int = len(self._points) - 1
        coord: Position = data['position']

        marker_data = {
            'ball_id': id,
            'coord': coord,
            'size': 2,
            'colour': (1.0, 1.0, 0.0),
        }
        msg = {
            "topic": "Add marker",
            "data": marker_data,
        }
        await self._socketio.emit(
            event="to_neuronavigation",
            data=msg,
        )
