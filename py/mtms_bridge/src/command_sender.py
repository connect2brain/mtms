#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import logging
from typing import List

from mtms.kafka.listener import KafkaListener

# TODO: Add type hints to this class.
#
class CommandSender():
    """A class for sending commands to LabVIEW.

    """
    def __init__(self, kafka=None, server=None, topic_db=None, delay=0.1):
        """Initialize the command sender.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        server : MTMSConnection
            A connection to LabVIEW.
        topic_db : TopicDb
            A connection to the topic database.
        delay : float
            The delay (in seconds) between two consecutive runs of the command sender.
            Defaults to 0.1 seconds.
        """
        self._kafka = kafka
        self._server = server
        self._delay = delay

        # TODO: Implement proper propagation of 'stimulation.set_coil_at_target' to LabVIEW.
        #       It's currently marked as a 'command' type topic, which means that it is
        #       handled by this class. However, the other commands are, e.g., 'stimulate'
        #       and 'recharge', which directly change the state of the corresponding button in LabVIEW.
        #       With 'stimulation.set_coil_at_target', we might want to define a mapping from
        #       the topic name to 'coil_at_target' variable name to make it analogous to
        #       'stimulate' and 'recharge'. However, there are alternatives, e.g., parsing the
        #       variable name from the Kafka topic, which would then benefit from being renamed to
        #       'stimulation.set.coil_at_target'. Overall, I think this needs some more thinking
        #       before implementing.
        #
        self._command_topics = topic_db.get_topics(type='command')

        self._connected = False

        self._setup_background_tasks()

    def _setup_background_tasks(self) -> None:
        """Set up background tasks, namely, a Kafka listener for each topic of the type 'command'.

        """
        self.background_tasks: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=topic,
                callback=self._send_command,
            ) for topic in self._command_topics
        ]

    async def _send_command(self, topic, value):
        """Send a message of the type 'command' to LabVIEW.

        Parameters
        ----------
        topic : str
            The topic in which the message is received.
        value : number or None
            The value for the message in the topic. Only relevant if the message type is 'parameter'.
        """
        msg_str = "command {}".format(topic)

        if self._connected:
            logging.info("[Try ] Send {}".format(msg_str))

            sent = self._server.send(msg_type='command', param1=topic, param2=value)

            if sent:
                logging.info("[Done] Send {}".format(msg_str))
            else:
                logging.info("[Fail] Send {}".format(msg_str))
        else:
            logging.info("[Done] {} received, not connected.".format(msg_str))

    async def run(self):
        """Keep track of the status of the connection to LabVIEW.

        """
        asyncio.current_task().name = "command-sender"
        try:
            while True:
                if not self._connected and self._server.is_connected():
                    self._connected = True

                if self._connected and not self._server.is_connected():
                    self._connected = False

                await asyncio.sleep(self._delay)

        except asyncio.CancelledError as e:
            logging.info("Cancelled task {}".format(asyncio.current_task().name))
            raise e

        # General exception handling is needed here so that exceptions within asyncio coroutines are logged properly.
        except Exception as e:
            logging.exception(e)
