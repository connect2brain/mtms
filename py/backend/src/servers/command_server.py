#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from typing import List

from flask_socketio import SocketIO

from mtms.kafka.kafka import Kafka
from mtms.db.topic_db import TopicDb

class CommandServer:
    """A server for receiving commands from the clients.

    """

    _COMMAND_EVENT: str = 'command'
    _COMMAND_TOPIC_TYPE: str = 'command'

    def __init__(self, kafka: Kafka, socketio: SocketIO, topic_db: TopicDb) -> None:
        """Initialize the command server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            A SocketIO object to which the event listeners are added.
        topic_db
            A TopicDb object to communicate with the topic database.
        """
        self._kafka: Kafka = kafka
        self._socketio: SocketIO = socketio
        self._topic_db: TopicDb = topic_db

        self._commands: List[str] = self._topic_db.get_topics_by_type(self._COMMAND_TOPIC_TYPE)

        socketio.on_event(self._COMMAND_EVENT, self._send_command)

    def _send_command(self, command: str) -> None:
        """Send a command via Kafka.

        Parameters
        ----------
        command
            The command to be sent, also the name of the Kafka topic in which the command is published.
        """
        assert command in self._commands, "{} is not a valid command".format(command)

        self._kafka.produce(
            topic=command,
            value=bytes(str(command), encoding='utf8')
        )
