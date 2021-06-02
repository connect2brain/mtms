#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import pytest
import sys
import time
from typing import List

sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../src")

from mtms.mocks.mock_kafka import MockKafka
from mtms.mocks.mock_topic_db import MockTopicDb
from mtms.mocks.mock_socket_io import MockSocketIO

from servers.command_server import CommandServer

def test_command_server() -> None:
    """Tests CommandServer class.

    """

    # Set up CommandServer.
    broadcasted: List[str] = []

    kafka: MockKafka = MockKafka()
    socketio: MockSocketIO = MockSocketIO(broadcasted=broadcasted)
    topic_db: MockTopicDb = MockTopicDb()

    server: CommandServer = CommandServer(
        kafka=kafka,
        socketio=socketio,
        topic_db=topic_db,
    )

    # Test that connecting to the command server does not broadcast anything.
    socketio.simulate_event('connect')

    assert len(broadcasted) == 0

    # Smoke test that a command sent via SocketIO reaches Kafka.
    socketio.simulate_event('command', 'stimulate')

    consumer = kafka.get_consumer('stimulate')

    message = consumer.poll()
    value = consumer.message_to_value(message)

    assert value == b'stimulate'
