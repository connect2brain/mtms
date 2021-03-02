#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time

import flask_socketio

class ParameterServer:
    """A server for sending parameter values to clients.

    The contracts for this server:
    - When a new client connects, sends the current value of all parameters to the client.
    - When a parameter value in Kafka changes, sends the new value to all connected clients.
    """

    _PARAMETER_TOPIC_TYPE = 'parameter'
    _UPDATE_PARAMETER_EVENT = 'update_parameter'

    def __init__(self, kafka=None, socketio=None, topic_db=None):
        """Initialize the parameter server.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        socketio : flask_socketio.SocketIO
            A SocketIO object to which the event listeners are added.
        topic_db : TopicDb
            A TopicDb object to communicate with the topic database.
        """
        self._parameters = {}
        self._kafka = kafka
        self._socketio = socketio
        self._topic_db = topic_db

        self._parameter_topics = self._topic_db.get_topics_by_type(self._PARAMETER_TOPIC_TYPE)
        self._setup_listeners()

        socketio.on_event('connect', self._send_parameters_on_connect)
        socketio.on_event(self._UPDATE_PARAMETER_EVENT, self._set_parameter_to_kafka)

    def _setup_listeners(self):
        """Setup up a Kafka listener for each topic.

        """
        self._listeners = [
            self._kafka.get_listener(
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
        if topic not in self._parameters:
            return None

        value = self._parameters[topic]
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
        """Set a parameter received from a client to Kafka.

        Called when a connected client updates a parameter value.

        Parameters
        ----------
        data : dict
            A dict consisting of 'name' and 'value' keys. Value for 'name' tells the name of the
            parameter that is updated, and value for 'value' tells the new value.
        """
        name = data['name']
        value = data['value']
        assert name in self._parameter_topics, "{} is not a valid parameter name".format(name)

        # XXX: Recreating the producer each time is slow with PyKafka, revisit after changing
        #   the Kafka library to a faster one.
        producer = self._kafka.get_producer(topic=name)
        producer.produce(bytes(str(value), encoding='utf8'))
