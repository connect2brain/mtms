#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import itertools
import os
import sys

import asyncio
import nest_asyncio
import socketio as sio
import uvicorn
from engineio.payload import Payload

from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener
from mtms.db.topic_db import TopicDb

from servers.eeg_server import EegServer
from servers.command_server import CommandServer
from servers.parameter_server import ParameterServer
from servers.planner_server import PlannerServer
from servers.state_server import StateServer

# Patch asyncio library to allow running Uvicorn server inside the event loop
# (= main function). For more information, see, e.g.:
#
# https://medium.com/@vyshali.enukonda/how-to-get-around-runtimeerror-this-event-loop-is-already-running-3f26f67e762e
#
nest_asyncio.apply()

# Increase max_decode_packets from the default (16) to 50, otherwise causes
# the error "Too many packets in payload" when streaming EEG data.
#
# TODO: Consider streaming in larger packets to be able to use the default value.
#
Payload.max_decode_packets = 50

# Read environment variables
backend_port: int = int(os.getenv("BACKEND_PORT"))
eeg_buffer_length: int = int(os.getenv("BACKEND_EEG_BUFFER_LENGTH", 8192))

# Create connection to Kafka
kafka: Kafka = Kafka()

# Create connection to database
topic_db: TopicDb = TopicDb()

# Create app
socketio = sio.AsyncServer(async_mode='asgi', cors_allowed_origins='*')
app = sio.ASGIApp(socketio)

# Create server for EEG data
eeg_server: EegServer = EegServer(
    kafka=kafka,
    socketio=socketio,
    eeg_buffer_length=eeg_buffer_length,
)

# Create server for parameters
parameter_server: ParameterServer = ParameterServer(
    kafka=kafka,
    socketio=socketio,
    topic_db=topic_db,
)

# Create server for commands
command_server: CommandServer = CommandServer(
    kafka=kafka,
    socketio=socketio,
    topic_db=topic_db,
)

# Create server for state
state_server: StateServer = StateServer(
    kafka=kafka,
    socketio=socketio,
    topic_db=topic_db,
)

# Create server for planner
planner_server: PlannerServer = PlannerServer(
    kafka=kafka,
    socketio=socketio
)

async def main() -> None:
    # TODO: Does not currently log the name of the task that writes the log line. That would be good to have.
    #       Here's a pointer for implementing it using logging filters and the task name:
    #
    #       https://docs.python.org/3/howto/logging-cookbook.html#using-filters-to-impart-contextual-information
    #
    task: KafkaListener
    for task in itertools.chain(
        eeg_server.background_tasks,
        parameter_server.background_tasks,
        planner_server.background_tasks,
        state_server.background_tasks,
    ):
        asyncio.create_task(task.run())

    uvicorn.run(app, port=backend_port, host='0.0.0.0', loop='asyncio')

# Run app
if __name__ == '__main__':
    asyncio.run(main(), debug=True)
