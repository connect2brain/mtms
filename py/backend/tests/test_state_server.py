#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import os
import sys
import time
from typing import List, Union

import pytest

sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../src")

from mtms.kafka.listener import KafkaListener
from mtms.mocks.mock_kafka import MockKafka
from mtms.mocks.mock_topic_db import MockTopicDb
from mtms.mocks.mock_socket_io import MockSocketIO

from servers.state_server import StateServer

SocketIOData = Union[str, dict]

@pytest.mark.asyncio
async def test_state_server(mocker) -> None:
    """Tests StateServer class.

    """

    # Set up StateServer.
    broadcasted: List[SocketIOData] = []

    kafka: MockKafka = MockKafka()
    socketio: MockSocketIO = MockSocketIO(
        broadcasted=broadcasted,
    )
    topic_db: MockTopicDb = MockTopicDb()

    server: StateServer = StateServer(
        kafka=kafka,
        socketio=socketio,
        topic_db=topic_db,
    )

    task: KafkaListener
    for task in server.background_tasks:
        asyncio.create_task(task.run())

    # Test that connecting to the state server does not broadcast anything.
    await socketio.simulate_event('connect')

    assert len(broadcasted) == 0

    # Produce a value for a state variable in Kafka.
    kafka.produce(
        topic='error_code',
        value=11,
    )

    await asyncio.sleep(1)

    # Test that the state value is broadcast to all clients.
    assert len(broadcasted) == 1
    assert broadcasted[0] == {
        'event': 'update_state',
        'data': {
            'state_variable': 'error_code',
            'value': 11,
        },
    }

    # Produce a value for a parameter variable in Kafka.
    kafka.produce(
        topic='intensity',
        value=123,
    )

    await asyncio.sleep(1)

    # Test that the parameter value is not broadcast to the clients by the state server.
    assert len(broadcasted) == 1
