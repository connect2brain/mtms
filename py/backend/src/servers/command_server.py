#!/usr/bin/env python3
# -*- coding: utf-8 -*-

class CommandServer:
    """A server for receiving commands from the clients.

    """

    _COMMAND_EVENT = 'command'
    _COMMAND_TOPIC_TYPE = 'command'

    def __init__(self, kafka=None, socketio=None, topic_db=None):
        """Initialize the command server.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        socketio : flask_socketio.SocketIO
            A SocketIO object to which the event listeners are added.
        topic_db : TopicDb
            A TopicDb object to communicate with the topic database.
        """
        self._kafka = kafka
        self._socketio = socketio
        self._topic_db = topic_db

        self._commands = self._topic_db.get_topics_by_type(self._COMMAND_TOPIC_TYPE)

        socketio.on_event(self._COMMAND_EVENT, self._send_command)

    def _send_command(self, command):
        """Send a command via Kafka.

        Parameters
        ----------
        command : str
            The command to be sent, also the name of the Kafka topic in which the command is published.
        """
        assert command in self._commands, "{} is not a valid command".format(command)

        # XXX: Recreating the producer for each command is slow with PyKafka, revisit after changing
        #   the Kafka library to a faster one.
        producer = self._kafka.get_producer(topic=command)
        producer.produce(bytes(str(command), encoding='utf8'))
