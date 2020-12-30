#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time

from flask_socketio import emit
from kafka.listener import KafkaListener
from kafka.util import get_kafka_producer

from db.topics import TopicDb

class ParameterServer:
    """A server for sending parameter values to clients.

    The contracts for this server:
    - When a new client connects, sends the current value of all parameters to the client.
    - When a parameter value in Kafka changes, sends the new value to all connected clients.

    Only serves clients connected to the parameter namespace.
    """

    _PARAMETER_NAMESPACE = '/parameters'
    _PARAMETER_TOPIC_TYPE = 'parameter'
    _UPDATE_PARAMETER_EVENT = 'update_parameter'

    def __init__(self, socketio=None):
        """Initialize the parameter server.

        Parameters
        ----------
        socketio : flask_socketio.SocketIO
            A SocketIO object to which the event listeners are added.
        """
        self._parameters = {}
        self._socketio = socketio

        topic_db = TopicDb()
        self._parameter_topics = topic_db.get_topics_by_type(self._PARAMETER_TOPIC_TYPE)
        self._setup_listeners()

        socketio.on_event('connect', self._send_parameters_on_connect, namespace=self._PARAMETER_NAMESPACE)
        socketio.on_event(self._UPDATE_PARAMETER_EVENT, self._set_parameter_to_kafka, namespace=self._PARAMETER_NAMESPACE)

    def _setup_listeners(self):
        """Setup up a Kafka listener for each topic.

        """
        self._listeners = [
            KafkaListener(
                topic=topic,
                callback=self._get_parameter_from_kafka,
            ) for topic in self._parameter_topics
        ]

        for listener in self._listeners:
            listener.start()

    def _send_parameter(self, topic=None, broadcast=None):
        """Send the parameter value in the given topic to one or several clients.

        Parameters
        ----------
        topic : str
            The topic which contains the parameter to be sent.
        broadcast : bool
            If True, the parameter value is sent to all clients. If False, the value is sent
            only to the client in the context.
        """
        value = self._parameters[topic]
        data = {
            'name': topic,
            'value': value,
        }
        if broadcast:
            self._socketio.emit(self._UPDATE_PARAMETER_EVENT, data, namespace=self._PARAMETER_NAMESPACE)
        else:
            emit(self._UPDATE_PARAMETER_EVENT, data, namespace=self._PARAMETER_NAMESPACE)

    def _send_parameters_on_connect(self):
        """Send all parameters to the client.

        Called when a new client connects.
        """
        for topic in self._parameter_topics:
            self._send_parameter(
                topic=topic,
                broadcast=False
            )

    def _get_parameter_from_kafka(self, topic, value):
        """Update the parameter internally and broadcast to all connected clients.

        Called when a Kafka listener triggers.

        Parameters
        ----------
        topic : str
            The topic for the new parameter value.
        value : number
            The new value.
        """
        self._parameters[topic] = value
        self._send_parameter(
            topic=topic,
            broadcast=True
        )

    def _set_parameter_to_kafka(self, data):
        name = data['name']
        value = data['value']
        assert name in self._parameter_topics, "{} is not a valid parameter name".format(name)

        producer = get_kafka_producer(topic=name)
        producer.produce(bytes(str(value), encoding='utf8'))
