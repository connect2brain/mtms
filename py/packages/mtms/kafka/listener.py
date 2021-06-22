#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# TODO: Type hints are mostly missing from this file.

import asyncio
import logging
import sys
import time
from threading import Thread
from typing import Callable

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class KafkaListener():
    """A wrapper around KafkaConsumer that pushes the data produced into the given topic to a callback.

    """
    def __init__(self, kafka=None, topic=None, callback=None, delay=0.1, verbose=True):
        """Initialize the listener.

        Parameters
        ----------
        kafka : Kafka
            A connection to Kafka.
        topic : str
            The name of the Kafka topic.
        callback : callable
            The function that is called when new data are produced into the topic.
        delay : float
            The delay (in seconds) between two consecutive runs of the listener.
            Defaults to 0.1 seconds.
        verbose : boolean
            If True, logs every new message received. Otherwise logs only every 100th
            message. Defaults to True.
        """
        self._kafka = kafka
        self._topic = topic
        self._callback = callback
        self._delay = delay
        self._verbose = verbose

        self._consumer = self._kafka.get_consumer(
            topic=topic,
            timeout=0,
        )
        self._msgs_received = 0

    def reset(self):
        """Reset the listener to re-read the last message in the topic.

        """
        self._consumer.reset()

    def _read_value(self):
        """Read one message from Kafka, return the value.

        Notes
        -----
        Returns None if there are no new messages.
        """
        value = None
        raw_message = self._consumer.poll()
        if raw_message is not None:
            value = self._consumer.message_to_value(raw_message)
            if self._verbose:
                logging.info("A new message received in topic '{topic}', value: {value}".format(
                    topic=self._topic,
                    value=value,
                ))
            else:
                self._msgs_received += 1
                if self._msgs_received == 100:
                    self._msgs_received = 0
                    logging.info("100 new messages have been received in topic '{}'".format(self._topic))

        return value

    async def run(self):
        """Read messages from Kafka and call the callback function with the new data.

        """
        asyncio.current_task().name = "kafka-listener-{}".format(self._topic)
        try:
            while True:
                value = self._read_value()
                if value is not None:
                    await self._callback(self._topic, value)
                await asyncio.sleep(self._delay)

        except asyncio.CancelledError as e:
            logging.info("Cancelled task {}".format(asyncio.current_task().name))
            raise e

        # General exception handling is needed here so that exceptions within asyncio coroutines are logged properly.
        except Exception as e:
            logging.exception(e)
