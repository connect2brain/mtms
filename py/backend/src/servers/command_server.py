#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from typing import List

from socketio import AsyncServer

from mtms.kafka.kafka import Kafka
from mtms.db.topic_db import TopicDb

class CommandServer:
    """A server for receiving commands from the clients.

    """

    _COMMAND_EVENT: str = 'command'
    _COMMAND_TOPIC_TYPE: str = 'command'

    def __init__(self, kafka: Kafka, socketio: AsyncServer, topic_db: TopicDb) -> None:
        """Initialize the command server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            An AsyncServer object to which the event listeners are added.
        topic_db
            A TopicDb object to communicate with the topic database.
        """
        self._kafka: Kafka = kafka
        self._socketio: AsyncServer = socketio
        self._topic_db: TopicDb = topic_db

        self._commands: List[str] = self._topic_db.get_topics(type=self._COMMAND_TOPIC_TYPE)

        socketio.on(
            event=self._COMMAND_EVENT,
            handler=self._send_command,
        )

    def _send_command(self, client_id: str, data: str) -> None:
        """Send a command via Kafka.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        data
            The command to be sent, also the name of the Kafka topic in which the command is published.
        """
        # XXX: Sending the plain command as the data of the message is not very clean, at least it
        #      should be wrapped inside a JSON dict or such. One example of the uncleanliness is below,
        #      where the generic-sounding variable "data" is implicitly assumed to be very specific.
        assert data in self._commands, "{} is not a valid command".format(data)

        self._kafka.produce(
            topic=data,
            value=bytes(str(data), encoding='utf8')
        )
