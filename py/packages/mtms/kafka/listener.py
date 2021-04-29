#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import sys
import time
from threading import Thread

import pykafka.exceptions

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class KafkaListener(Thread):
    """A wrapper around KafkaConsumer that pushes the data produced into the given topic to a callback.

    """
    def __init__(self, kafka=None, topic=None, callback=None):
        """Initialize the listener.

        Parameters
        ----------
        kafka : Kafka
            A connection to Kafka.
        topic : str
            The topic name.
        callback : function
            The function that is called when new data are produced in the topic.
        """
        Thread.__init__(self)
        self._kafka = kafka
        self._topic = topic
        self._callback = callback

        self._thread_name = 'kafka_listener_' + topic
        self._consumer = self._kafka.get_consumer(topic=topic)
        self.daemon = True

    def reset(self):
        """Reset the listener to re-read the last message in the topic.

        """
        self._kafka.reset_consumer(consumer=self._consumer)

    def _read_value(self):
        """Read one message from Kafka, return the value.

        Notes
        -----
        Returns None if there are no new messages.
        """
        value = None
        try:
            raw_message = self._consumer.consume()
            if raw_message is not None:
                value = raw_message.value
                logging.info("A new message received in topic '{topic}', value: {value}".format(
                    topic=self._topic,
                    value=value,
                ))
        except pykafka.exceptions.SocketDisconnectedError as e:
            sys.stderr.write("[ERROR] Kafka socket disconnected. Reason: '{}'".format(e))

        return value

    def run(self, sleep=time.sleep, sleep_time=0.1):
        """Read messages from Kafka and call the callback function with the new data.

        Parameters
        ----------
        sleep : callable
            The sleep function used to wait a set amount of time between two consecutive
            runs of the listener.

            Defaults to time.sleep, but allows custom sleep functions. The reason for that
            is that Socket.IO requires using its own sleep function for emitting to work
            from threads.
        sleep_time : number
            The time in seconds that is waited between consecutive runs of the listener.

            Defaults to 0.1 seconds.
        """
        logging.info("Starting thread " + self._thread_name)
        while True:
            value = self._read_value()
            if value is not None:
                self._callback(self._topic, value)
            sleep(sleep_time)
