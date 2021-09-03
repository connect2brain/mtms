#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import json
import logging
from typing import List

from mtms.kafka.listener import KafkaListener

from mtms.common.constants import KAFKA_COMMAND_SET_STIMULATION_PARAMETERS, STIMULATION_PARAMETERS
from mtms.common.types import Intensity, Isi, StimulationParameters

# TODO: Add type hints to this class.
#
class ParameterSender():
    """A class for sending parameters to LabVIEW.

    """
    def __init__(self, kafka=None, server=None, topic_db=None, delay=0.1):
        """Initialize the parameter sender.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        server : MTMSConnection
            A connection to LabVIEW.
        topic_db : TopicDb
            A connection to the topic database.
        delay : float
            The delay (in seconds) between two consecutive runs of the parameter sender.
            Defaults to 0.1 seconds.
        """
        self._kafka = kafka
        self._server = server
        self._delay = delay

        self._parameters = None
        self._connected = False

        self._setup_background_tasks()

    def _setup_background_tasks(self) -> None:
        """Set up background tasks, namely, a Kafka listener for each topic.

        """
        self.background_tasks: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=KAFKA_COMMAND_SET_STIMULATION_PARAMETERS,
                callback=self._update_and_send_parameters,
            )
        ]

    async def _update_and_send_parameters(self, topic: str, value: StimulationParameters):
        """Update the parameters after receiving a message from Kafka and send them
        to the client if connected.

        Parameters
        ----------
        topic
            The topic in which the message is received from Kafka.
        value
            The value for the message in the topic.

            See type StimulationParameters for the specification.
        """
        logging.info("New stimulation parameters received.")

        self._parameters = json.loads(value)
        await self._send_parameters()

    async def _send_parameters(self):
        """Send all parameters to LabVIEW via messages of type 'parameter'. If the parameters
        have not been received from Kafka, do nothing.

        """
        if self._parameters is None:
            return

        for parameter in STIMULATION_PARAMETERS:
            parameter_value = self._parameters[parameter]

            msg_str: str = "parameter {} = {}".format(parameter, str(parameter_value))

            if self._connected:
                logging.info("[Try ] Send {}".format(msg_str))

                sent = self._server.send(msg_type='parameter', param1=parameter, param2=parameter_value)

                if sent:
                    logging.info("[Done] Send {}".format(msg_str))
                else:
                    logging.info("[Fail] Send {}".format(msg_str))
            else:
                logging.info("[Done] {} received, not connected.".format(msg_str))

    async def run(self):
        """Resend the latest parameter values when a new connection is formed.

        """
        asyncio.current_task().name = "parameter-sender"
        try:
            while True:
                if not self._connected and self._server.is_connected():
                    self._connected = True

                    msg_str = "Resending parameters."
                    logging.info("[Try ] {}".format(msg_str))

                    await self._send_parameters()

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
