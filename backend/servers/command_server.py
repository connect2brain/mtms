#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from kafka.util import get_kafka_producer

from db.topics import TopicDb

class CommandServer:
    """A server for receiving commands from the clients.

    Only serves clients connected to the command namespace.
    """

    _COMMAND_NAMESPACE = '/commands'
    _COMMAND_EVENT = 'command'
    _COMMAND_TOPIC_TYPE = 'command'

    def __init__(self, socketio=None):
        """Initialize the command server.

        Parameters
        ----------
        socketio : flask_socketio.SocketIO
            A SocketIO object to which the event listeners are added.
        """
        self._socketio = socketio

        topic_db = TopicDb()
        self._commands = topic_db.get_topics_by_type(self._COMMAND_TOPIC_TYPE)

        socketio.on_event(self._COMMAND_EVENT, self._send_command, namespace=self._COMMAND_NAMESPACE)

    def _send_command(self, command):
        """Send a command via Kafka.

        Parameters
        ----------
        command : str
            The command to be sent, also the name of the Kafka topic in which the command is published.
        """
        assert command in self._commands, "{} is not a valid command".format(command)
        producer = get_kafka_producer(topic=command)
        producer.produce(bytes(str(command), encoding='utf8'))
