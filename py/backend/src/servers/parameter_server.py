#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import time
from typing import Any, Dict, List

from socketio import AsyncServer

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

    def __init__(self, kafka: Kafka, socketio: AsyncServer, topic_db: TopicDb) -> None:
        """Initialize the parameter server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            An AsyncServer object to which the event listeners are added.
        topic_db
            A TopicDb object to communicate with the topic database.
        """
        self._parameters: Dict[str, float] = {}
        self._kafka: Kafka = kafka
        self._socketio: AsyncServer = socketio
        self._topic_db: TopicDb = topic_db

        self._parameter_topics: List[str] = self._topic_db.get_topics(type=self._PARAMETER_TOPIC_TYPE)

        socketio.on(
            event='connect',
            handler=self._send_parameters_on_connect,
        )
        socketio.on(
            event=self._UPDATE_PARAMETER_EVENT,
            handler=self._set_parameter_to_kafka,
        )

        self._setup_background_tasks()

    def _setup_background_tasks(self) -> None:
        """Set up background tasks, namely, a Kafka listener for each topic.

        """
        topic: str
        self.background_tasks: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=topic,
                callback=self._get_parameter_from_kafka,
            ) for topic in self._parameter_topics
        ]

    async def _send_parameter(self, client_id: str = None, topic: str) -> None:
        """Send the parameter value in the given topic to one or several Socket.IO clients.
        If the topic is not listed in the topic database, do nothing.

        Parameters
        ----------
        client_id
            The client id. If provided, the value is sent only to the client with that id.
            If not provided, the parameter value is sent to all clients.

            Defaults to None.
        topic
            The topic which contains the parameter to be sent.
        """
        if topic not in self._parameters:
            return None

        value: float = self._parameters[topic]
        data = {
            'name': topic,
            'value': value,
        }
        await self._socketio.emit(
            event=self._UPDATE_PARAMETER_EVENT,
            data=data,
            to=client_id,
        )

    async def _send_parameters_on_connect(self, client_id: str, environment: Dict[str, Any]) -> None:
        """Send all parameters to the connected Socket.IO client.

        Called when a new client connects.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        environment
            The WSGI environment provided by the AsyncServer.
        """
        topic: str
        for topic in self._parameter_topics:
            await self._send_parameter(
                client_id=client_id,
                topic=topic,
            )

    # TODO: Function naming in this class needs to be rethought -- it's a bit unclear
    #       currently.
    async def _get_parameter_from_kafka(self, topic: str, value: float) -> None:
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
        await self._send_parameter(
            topic=topic,
        )

    def _set_parameter_to_kafka(self, client_id: str, data: Dict[str, Any]):
        """Set a parameter received from a client to Kafka.

        Called when a connected client updates a parameter value.

        Parameters
        ----------
        client_id
            The client id provided by the AsyncServer.
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
