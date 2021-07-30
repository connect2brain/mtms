#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import itertools
import logging
import time

from mtms.db.topic_db import TopicDb
from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener

from mtms_connection import MTMSConnection
from state_receiver import StateReceiver
from parameter_command_sender import ParameterCommandSender

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# Initialization
kafka = Kafka()
topic_db = TopicDb()
server = MTMSConnection(is_server=True)

state_receiver = StateReceiver(kafka=kafka, server=server, topic_db=topic_db)
parameter_command_sender = ParameterCommandSender(kafka=kafka, server=server, topic_db=topic_db)

async def main() -> None:
    # TODO: Does not currently log the name of the task that writes the log line. That would be good to have.
    #       Here's a pointer for implementing it using logging filters and the task name:
    #
    #       https://docs.python.org/3/howto/logging-cookbook.html#using-filters-to-impart-contextual-information
    #
    #       See a similar concern in backend.
    #
    task: KafkaListener
    for task in itertools.chain(
        parameter_command_sender.background_tasks,
    ):
        asyncio.create_task(task.run())

    asyncio.create_task(parameter_command_sender.run())
    asyncio.create_task(server.run())
    asyncio.create_task(state_receiver.run())

    while True:
        await asyncio.sleep(1.0)

if __name__ == '__main__':
    asyncio.run(main(), debug=True)
