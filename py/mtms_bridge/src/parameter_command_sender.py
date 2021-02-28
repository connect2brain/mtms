#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from functools import partial
from threading import Thread

class ParameterCommandSender(Thread):
    """A class for sending parameters and commands to LabVIEW.

    """
    def __init__(self, kafka=None, server=None, topic_db=None):
        """Initialize the parameter and command sender.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        server : MTMSConnection
            A connection to LabVIEW.
        topic_db : TopicDb
            A connection to the topic database.
        """
        self._kafka = kafka
        self._server = server
        self._parameter_topics = topic_db.get_topics_by_type('parameter')
        self._command_topics = topic_db.get_topics_by_type('command')

        self._connected = False

        # Initialize listener for each topic.
        self._parameter_listeners = [
            self._kafka.get_listener(
                topic=topic,
                callback=partial(self._send, 'parameter'),
            ) for topic in self._parameter_topics
        ]

        self._command_listeners = [
            self._kafka.get_listener(
                topic=topic,
                callback=partial(self._send, 'command'),
            ) for topic in self._command_topics
        ]

        # Start listener threads.
        for listener in self._parameter_listeners + self._command_listeners:
            listener.start()

        Thread.__init__(self)
        self.daemon = True

    def _send(self, msg_type, topic, value):
        """Send a message to LabVIEW.

        Parameters
        ----------
        msg_type : str
            The type of the message. Either 'parameter' or 'command'.
        topic : str
            The topic in which the message is received.
        value : number or None
            The value for the message in the topic. Only relevant if the message type is 'parameter'.
        """
        if msg_type == 'parameter':
            msg_str = "parameter {} = {}".format(topic, str(value))

        elif msg_type == 'command':
            msg_str = "command {}".format(topic)

        else:
            assert False, "Unknown message type"

        if self._connected:
            logging.info("[Try ] Send {}".format(msg_str))

            sent = self._server.send(msg_type=msg_type, param1=topic, param2=value)

            if sent:
                logging.info("[Done] Send {}".format(msg_str))
            else:
                logging.info("[Fail] Send {}".format(msg_str))
        else:
            logging.info("[Done] {} received, not connected.".format(msg_str))

    def run(self):
        """Reset parameter topics to resend the latest value when a new connection is formed.

        """
        while True:
            if not self._connected and self._server.is_connected():
                self._connected = True

                msg_str = "Resetting parameter topics."
                logging.info("[Try ] {}".format(msg_str))
                for listener in self._parameter_listeners:
                    listener.reset()
                logging.info("[Done] {}".format(msg_str))

            if self._connected and not self._server.is_connected():
                self._connected = False
