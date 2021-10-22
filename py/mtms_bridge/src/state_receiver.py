#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import logging

# TODO: Add type hints to this class.
#
class StateReceiver():
    """A class for receiving state from LabVIEW.

    """
    def __init__(self, kafka=None, server=None, topic_db=None, delay=0.1):
        """Initialize the state receiver.

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
        self._state_topics = topic_db.get_topics(type='state')

    async def run(self):
        """Read state messages from the server and pass them on to Kafka.

        """
        asyncio.current_task().name = "state-receiver"
        try:
            while True:
                msg_type, param1, param2 = self._server.receive()
                if msg_type == 'state':
                    logging.info("[Done] Receive state: {} = {}".format(param1, str(param2)))

                    self._kafka.produce(
                        topic=param1,
                        value=bytes(str(param2), encoding='utf8')
                    )
                await asyncio.sleep(self._delay)

        except asyncio.CancelledError as e:
            logging.info("Cancelled task {}".format(asyncio.current_task().name))
            raise e

        # General exception handling is needed here so that exceptions within asyncio coroutines are logged properly.
        except Exception as e:
            logging.exception(e)
