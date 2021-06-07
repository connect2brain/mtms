#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
from typing import Any, Dict

import flask_socketio
from flask_socketio import SocketIO

from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener
from mtms.db.topic_db import TopicDb

class ParameterServer:
    """A server for sending parameter values to clients.

    The contracts for this server:
    - When a new client connects, sends the current value of all parameters to the client.
    - When a parameter value in Kafka changes, sends the new value to all connected clients.
    """

    _PARAMETER_TOPIC_TYPE: str = 'parameter'
    _UPDATE_PARAMETER_EVENT: str = 'update_parameter'

    def __init__(self, kafka: Kafka, socketio: SocketIO, topic_db: TopicDb) -> None:
        """Initialize the parameter server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            A SocketIO object to which the event listeners are added.
        topic_db
            A TopicDb object to communicate with the topic database.
        """
        self._parameters: Dict[str, float] = {}
        self._kafka: Kafka = kafka
        self._socketio: SocketIO = socketio
        self._topic_db: TopicDb = topic_db

        self._parameter_topics: List[str] = self._topic_db.get_topics(type=self._PARAMETER_TOPIC_TYPE)
        self._setup_listeners()

        socketio.on_event('connect', self._send_parameters_on_connect)
        socketio.on_event(self._UPDATE_PARAMETER_EVENT, self._set_parameter_to_kafka)

    def _setup_listeners(self) -> None:
        """Setup up a Kafka listener for each topic.

        """
        topic: str
        self._listeners: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=topic,
                callback=self._get_parameter_from_kafka,
            ) for topic in self._parameter_topics
        ]

        listener: KafkaListener
        for listener in self._listeners:
            listener.start()

    def _send_parameter(self, topic: str, broadcast: bool) -> None:
        """Send the parameter value in the given topic to one or several Socket.IO clients.
        If the topic is not listed in the topic database, do nothing.

        Parameters
        ----------
        topic
            The topic which contains the parameter to be sent.
        broadcast
            If True, the parameter value is sent to all clients. If False, the value is sent
            only to the client in the context.
        """
        if topic not in self._parameters:
            return None

        value: float = self._parameters[topic]
        data = {
            'name': topic,
            'value': value,
        }
        if broadcast:
            # XXX: SocketIO object's emit function broadcasts the event, whereas flask_socketio.emit
            #   sends it only to a single client if 'broadcast' argument is not specified. This convention
            #   seems a bit confusing and complicates testing.
            self._socketio.emit(self._UPDATE_PARAMETER_EVENT, data)
        else:
            flask_socketio.emit(self._UPDATE_PARAMETER_EVENT, data)

    def _send_parameters_on_connect(self) -> None:
        """Send all parameters to the connected Socket.IO client.

        Called when a new client connects.
        """
        topic: str
        for topic in self._parameter_topics:
            self._send_parameter(
                topic=topic,
                broadcast=False,
            )

    # TODO: Function naming in this class needs to be rethought -- it's a bit unclear
    #       currently.
    def _get_parameter_from_kafka(self, topic: str, value: float) -> None:
        """Update the parameter internally and broadcast to all connected clients.

        Called when a Kafka listener triggers.

        Parameters
        ----------
        topic
            The topic for the new parameter value.
        value
            The new value.
        """
        self._parameters[topic] = value
        self._send_parameter(
            topic=topic,
            broadcast=True
        )

    def _set_parameter_to_kafka(self, data: Dict[str, Any]):
        """Set a parameter received from a client to Kafka.

        Called when a connected client updates a parameter value.

        Parameters
        ----------
        data
            A dict consisting of 'name' and 'value' keys. Value for 'name' tells the name of the
            parameter that is updated, and value for 'value' tells the new value.
        """
        name: str = data['name']
        value: float = data['value']
        assert name in self._parameter_topics, "{} is not a valid parameter name".format(name)

        self._kafka.produce(
            topic=name,
            value=bytes(str(value), encoding='utf8')
        )
