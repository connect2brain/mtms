#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import itertools
import logging
import os
import time

from mtms.db.topic_db import TopicDb
from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener

from mtms_connection import MTMSConnection
from state_receiver import StateReceiver
from parameter_sender import ParameterSender
from command_sender import CommandSender

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

port = int(os.getenv("MTMS_BRIDGE_PORT"))

# Initialization
kafka = Kafka()
topic_db = TopicDb()
server = MTMSConnection(port=port, is_server=True)

state_receiver = StateReceiver(kafka=kafka, server=server, topic_db=topic_db)
parameter_sender = ParameterSender(kafka=kafka, server=server, topic_db=topic_db)
command_sender = CommandSender(kafka=kafka, server=server, topic_db=topic_db)

async def main() -> None:
    # XXX: This causes the asyncio warnings for some coroutines freezing or blocking
    #      the event loop to disappear. However, the proper solution would be to fix
    #      the freezing in the first place. There is an issue for this, see:
    #
    #      https://github.com/connect2brain/project-louhi/issues/228
    #
    asyncio.get_event_loop().slow_callback_duration = 1.0

    # TODO: Does not currently log the name of the task that writes the log line. That would be good to have.
    #       Here's a pointer for implementing it using logging filters and the task name:
    #
    #       https://docs.python.org/3/howto/logging-cookbook.html#using-filters-to-impart-contextual-information
    #
    #       See a similar concern in backend.
    #
    task: KafkaListener
    for task in itertools.chain(
        parameter_sender.background_tasks,
        command_sender.background_tasks,
    ):
        asyncio.create_task(task.run())

    asyncio.create_task(parameter_sender.run())
    asyncio.create_task(command_sender.run())
    asyncio.create_task(server.run_server())
    asyncio.create_task(state_receiver.run())

    while True:
        await asyncio.sleep(1.0)

if __name__ == '__main__':
    asyncio.run(main(), debug=True)
