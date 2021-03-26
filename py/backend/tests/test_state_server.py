#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import pytest
import sys
import time

sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../src")

from mtms.mocks.mock_kafka import MockKafka
from mtms.mocks.mock_topic_db import MockTopicDb
from mtms.mocks.mock_socket_io import MockSocketIO

from servers.state_server import StateServer

def test_state_server(mocker):
    """Tests StateServer class.

    """

    # Set up StateServer.
    broadcasted = []

    kafka = MockKafka()
    socketio = MockSocketIO(broadcasted=broadcasted)
    topic_db = MockTopicDb()

    server = StateServer(kafka=kafka, socketio=socketio, topic_db=topic_db)

    # Test that connecting to the state server does not broadcast anything.
    socketio.simulate_event('connect')

    assert len(broadcasted) == 0

    # Produce a value for a state variable in Kafka.
    producer = kafka.get_producer('error_code')
    producer.produce(11)

    time.sleep(1)

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
    producer = kafka.get_producer('intensity')
    producer.produce(123)

    time.sleep(1)

    # Test that the parameter value is not broadcast to the clients by the state server.
    assert len(broadcasted) == 1
