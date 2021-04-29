#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import json
from functools import partial

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class PlannerServer:
    """A backend server for the planner.

    """

    _NEURONAVIGATION_TOPICS = [
        'Set cross focal point',
    ]
    _COMMAND_ADD_POINT = 'point.add'

    def __init__(self, kafka=None, socketio=None):
        """Initialize the planner server.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        socketio : flask_socketio.SocketIO
            A SocketIO object to which the event listeners are added.
        """
        self._socketio = socketio
        self._kafka = kafka

        # Handlers for messages sent from neuronavigation.
        for topic in self._NEURONAVIGATION_TOPICS:
            socketio.on_event(
                topic,
                partial(self._handle_neuronavigation_message, topic),
            )

        # A handler for adding a new point in the front-end.
        socketio.on_event(
            self._COMMAND_ADD_POINT,
            self._add_point
        )

        # A Kafka listener for adding a new point.
        self._add_point_listener = self._kafka.get_listener(
            topic=self._COMMAND_ADD_POINT,
            callback=self._point_added,
        )
        self._socketio.start_background_task(target=lambda: self._add_point_listener.run(socketio.sleep))

        self.id_ = 0

    def _handle_neuronavigation_message(self, topic, data):
        """A handler for messages from neuronavigation.

        Parameters
        ----------
        topic : str
            The topic in which the message is sent.
        data : dict
            A dict consisting of the message.
        """
        # XXX: For now, broadcast the message with Socket.IO. Later, ensure that neuronavigation
        #   does not receive the message twice: first from its internal communication, and then
        #   from Socket.IO.
        self._socketio.emit(topic, data)

    def _add_point(self, data):
        """When a command is received from the front-end to add a new point, create the
        initial values for the point and pass the message to create a point to Kafka.

        Parameters
        ----------
        data : dict
            The data sent from the front-end with the command. Should consist of
            'position' key.
        """
        self.id_ += 1
        value = {
            'visible': False,
            'name': "Target-" + str(self.id_),
            'type': "Target",
            'comment': "",
            'position': data['position'],
        }

        # XXX: Recreating the producer for each command is slow with PyKafka, revisit after changing
        #   the Kafka library to a faster one.
        producer = self._kafka.get_producer(topic="point.add")
        producer.produce(bytes(json.dumps(value), encoding='utf8'))

    def _point_added(self, topic, data):
        """When a command is received from Kafka to add a new point, pass that command onto the
        front-end.

        Parameters
        ----------
        topic : str
            The name of the topic in which the command is sent.
        data : str
            A json dict consisting of the attributes of the new point.
        """
        data = json.loads(data)
        self._socketio.emit(topic, data)
