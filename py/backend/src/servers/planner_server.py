#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import json
from functools import partial
from typing import Any, Dict, List, Tuple, TypedDict

from flask_socketio import SocketIO

from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

Position = Tuple[float, float, float]

class AddPointData(TypedDict):
    position: Position

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

    _COMMAND_ADD_POINT: str = 'point.add'

    def __init__(self, kafka: Kafka, socketio: SocketIO) -> None:
        """Initialize the planner server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            A SocketIO object to which the event listeners are added.
        """
        self._socketio: SocketIO = socketio
        self._kafka: Kafka = kafka

        # Socket.IO event handlers

        socketio.on_event(
            'from_neuronavigation',
            self._handle_neuronavigation_message,
        )

        # A handler for adding a new point in the front-end.
        socketio.on_event(
            self._COMMAND_ADD_POINT,
            self._add_point
        )

        # A Kafka listener for adding a new point.
        self._add_point_listener: KafkaListener = KafkaListener(
            kafka=self._kafka,
            topic=self._COMMAND_ADD_POINT,
            callback=self._point_added,
        )
        self._add_point_listener.start()

        self.id_: int = 0

    def _handle_neuronavigation_message(self, msg: NeuroNavigationMessage) -> None:
        """A handler for messages from neuronavigation.

        Broadcasts the message received from neuronavigation.

        Parameters
        ----------
        msg
            The message received from neuronavigation, see type NeuroNavigationMessage for the structure.
        """
        topic: str = msg['topic']
        logging.info("Received a message from neuronavigation in topic '{}'".format(topic))

        # TODO: Consider broadcasting only the messages of interest.
        self._socketio.emit('from_neuronavigation', msg)

    def _add_point(self, data: AddPointData) -> None:
        """When a command is received from the front-end to add a new point, create the
        initial values for the point and pass the message to create a point to Kafka.

        Parameters
        ----------
        data
            The data sent from the front-end with the command. Should consist of
            'position' key, with a value that consists of a list of three floats.

            See type AddPointData for the specification.

            An example:

            {
                'position': [1.0, 2.0, 3.0],
            }
        """
        self.id_ += 1
        value: PointAddedData = {
            'visible': False,
            'name': "Target-" + str(self.id_),
            'type': "Target",
            'comment': "",
            'position': data['position'],
        }

        self._kafka.produce(
            topic=self._COMMAND_ADD_POINT,
            value=bytes(json.dumps(value), encoding='utf8')
        )

    def _point_added(self, topic: str, data: str):
        """When a command is received from Kafka to add a new point, pass that command onto the
        front-end.

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
        self._socketio.emit(topic, data)
