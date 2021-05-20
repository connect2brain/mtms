#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import os
import pytest
import sys
import time
from typing import Any, Dict, List

sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../src")

from mtms.mocks.mock_kafka import MockKafka
from mtms.mocks.mock_topic_db import MockTopicDb
from mtms.mocks.mock_socket_io import MockSocketIO

from servers.parameter_server import ParameterServer

def test_parameter_server(mocker) -> None:
    """Tests ParameterServer class.

    """

    # Set up ParameterServer.
    broadcasted: List[str] = []

    kafka: MockKafka = MockKafka()
    socketio: MockSocketIO = MockSocketIO(broadcasted=broadcasted)
    topic_db: MockTopicDb = MockTopicDb()

    server: ParameterServer = ParameterServer(kafka=kafka, socketio=socketio, topic_db=topic_db)

    # Patch SocketIO's emit function with our own, used when a new client connects.
    emitted_on_connect: List[Dict[str, Any]] = []
    def emit_on_connect(event: str, data: Dict[str, Any]):
        emitted_on_connect.append({
            'event': event,
            'data': data,
        })

    mocker.patch('servers.parameter_server.flask_socketio.emit', emit_on_connect)

    # Test that connecting to the parameter server does not emit parameters before they are initialized in Kafka.
    socketio.simulate_event('connect')

    assert len(emitted_on_connect) == 0
    assert len(broadcasted) == 0

    # Initialize a parameter in Kafka by producing a value for it.
    kafka.produce(
        topic='intensity',
        value=123,
    )

    time.sleep(1)

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
    assert len(emitted_on_connect) == 0

    socketio.simulate_event('connect')

    assert len(emitted_on_connect) == 1
    assert emitted_on_connect[0] == {
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

    time.sleep(1)

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
    emitted_on_connect.clear()

    socketio.simulate_event('connect')

    assert len(emitted_on_connect) == 2
    assert {e['data']['name'] for e in emitted_on_connect} == {'intensity', 'iti'}
