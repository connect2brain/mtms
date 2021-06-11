#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import os
import sys
import time
from typing import Any, Dict, List, Union

import pytest

sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../src")

from mtms.kafka.listener import KafkaListener
from mtms.mocks.mock_kafka import MockKafka
from mtms.mocks.mock_topic_db import MockTopicDb
from mtms.mocks.mock_socket_io import MockSocketIO

from servers.parameter_server import ParameterServer

SocketIOData = Union[str, dict]

@pytest.mark.asyncio
async def test_parameter_server(mocker) -> None:
    """Tests ParameterServer class.

    """

    # Set up ParameterServer.
    broadcasted: List[SocketIOData] = []
    sent_to_specific_client: List[SocketIOData] = []

    kafka: MockKafka = MockKafka()
    socketio: MockSocketIO = MockSocketIO(
        broadcasted=broadcasted,
        sent_to_specific_client=sent_to_specific_client,
    )
    topic_db: MockTopicDb = MockTopicDb()

    server: ParameterServer = ParameterServer(
        kafka=kafka,
        socketio=socketio,
        topic_db=topic_db,
    )

    task: KafkaListener
    for task in server.background_tasks:
        asyncio.create_task(task.run())

    # Test that connecting to the parameter server does not emit parameters before they are initialized in Kafka.
    await socketio.simulate_event('connect')

    assert len(sent_to_specific_client) == 0
    assert len(broadcasted) == 0

    # Initialize a parameter in Kafka by producing a value for it.
    kafka.produce(
        topic='intensity',
        value=123,
    )

    await asyncio.sleep(1)

    # Test that the parameter value is broadcast to all clients.
    assert len(broadcasted) == 1
    assert broadcasted[0] == {
        'event': 'update_parameter',
        'data': {
            'name': 'intensity',
            'value': 123,
        },
    }

    # Test that connecting to the parameter server now emits the parameter value to the client.
    assert len(sent_to_specific_client) == 0

    await socketio.simulate_event('connect')

    assert len(sent_to_specific_client) == 1
    assert sent_to_specific_client[0] == {
        'event': 'update_parameter',
        'data': {
            'name': 'intensity',
            'value': 123,
        },
    }

    # Initialize another parameter in Kafka.
    kafka.produce(
        topic='iti',
        value=500,
    )

    await asyncio.sleep(1)

    # Test that the parameter value is broadcast to all clients.
    assert len(broadcasted) == 2
    assert broadcasted[1] == {
        'event': 'update_parameter',
        'data': {
            'name': 'iti',
            'value': 500,
        },
    }

    # Test that connecting to the parameter server now emits both parameters.
    sent_to_specific_client.clear()

    await socketio.simulate_event('connect')

    assert len(sent_to_specific_client) == 2
    assert {e['data']['name'] for e in sent_to_specific_client} == {'intensity', 'iti'}
