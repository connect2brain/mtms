#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import logging
from functools import partial
from typing import List

from mtms.kafka.listener import KafkaListener

# TODO: Add type hints to this class.
#
class ParameterCommandSender():
    """A class for sending parameters and commands to LabVIEW.

    """
    def __init__(self, kafka=None, server=None, topic_db=None, delay=0.1):
        """Initialize the parameter and command sender.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        server : MTMSConnection
            A connection to LabVIEW.
        topic_db : TopicDb
            A connection to the topic database.
        delay : float
            The delay (in seconds) between two consecutive runs of the state receiver.
            Defaults to 0.1 seconds.
        """
        self._kafka = kafka
        self._server = server
        self._delay = delay

        self._parameter_topics = topic_db.get_topics(type='parameter')
        self._command_topics = topic_db.get_topics(type='command')

        self._connected = False

        self._setup_background_tasks()

    def _setup_background_tasks(self) -> None:
        """Set up background tasks, namely, a Kafka listener for each topic.

        """
        # Initialize listener for each topic.
        self._parameter_listeners = [
            KafkaListener(
                kafka=self._kafka,
                topic=topic,
                callback=self._send_parameter,
            ) for topic in self._parameter_topics
        ]

        self._command_listeners = [
            KafkaListener(
                kafka=self._kafka,
                topic=topic,
                callback=self._send_command,
            ) for topic in self._command_topics
        ]

        self.background_tasks: List[KafkaListener] = self._parameter_listeners + self._command_listeners

    async def _send_parameter(self, topic, value):
        """Send a message of type 'parameter' to LabVIEW.

        XXX: This wrapper around _send function is needed because functools.partial does not
             return a coroutine. Otherwise, _send_parameter would be equal to
             functools.partial(_send, 'parameter')

        Parameters
        ----------
        topic : str
            The topic in which the message is received.
        value : number or None
            The value for the message in the topic. Only relevant if the message type is 'parameter'.
        """
        await self._send('parameter', topic, value)

    async def _send_command(self, topic, value):
        """Send a message of type 'command' to LabVIEW.

        XXX: This wrapper around _send function is needed because functools.partial does not
             return a coroutine. Otherwise, _send_command would be equal to
             functools.partial(_send, 'command')

        Parameters
        ----------
        topic : str
            The topic in which the message is received.
        value : number or None
            The value for the message in the topic. Only relevant if the message type is 'parameter'.
        """
        await self._send('command', topic, value)

    async def _send(self, msg_type, topic, value):
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

    async def run(self):
        """Reset parameter topics to resend the latest value when a new connection is formed.

        """
        asyncio.current_task().name = "parameter-command-sender"
        try:
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

                await asyncio.sleep(self._delay)

        except asyncio.CancelledError as e:
            logging.info("Cancelled task {}".format(asyncio.current_task().name))
            raise e

        # General exception handling is needed here so that exceptions within asyncio coroutines are logged properly.
        except Exception as e:
            logging.exception(e)
