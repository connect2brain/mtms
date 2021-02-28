#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from threading import Thread

class StateReceiver(Thread):
    """A class for receiving state from LabVIEW.

    """
    def __init__(self, kafka=None, server=None, topic_db=None):
        """Initialize the state receiver.

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
        self._state_topics = topic_db.get_topics_by_type('state')

        self._producers = {}
        for topic in self._state_topics:
            # XXX: What if connection to Kafka is lost in the middle of this loop?
            producer = self._kafka.get_producer(topic=topic)
            self._producers[topic] = producer

        Thread.__init__(self)
        self.daemon = True

    def run(self):
        """Read state messages from the server and pass them on to Kafka.

        """
        while True:
            msg_type, param1, param2 = self._server.receive()
            if msg_type == 'state':
                logging.info("[Done] Receive state: {} = {}".format(param1, str(param2)))

                producer = self._producers[param1]
                # XXX: What if connection to Kafka is lost before this line? Should there be an exception handler?
                producer.produce(bytes(str(param2), encoding='utf8'))
