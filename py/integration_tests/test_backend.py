#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from typing import Dict

import asyncio
import pytest
import socketio

# XXX: The roundtrip max time cannot be set much lower than this for the tests to pass consistently. Why is that?
ROUNDTRIP_MAX_TIME: float = 10.0

@pytest.mark.asyncio
async def test_backend() -> None:
    """Test integration between backend and Kafka.

    """
    global ROUNDTRIP_MAX_TIME

    host: str = os.getenv("BACKEND_HOST")
    port: str = os.getenv("BACKEND_PORT")

    parameters: Dict[str, float] = {}
    state: Dict[str, float] = {}

    # Use asyncio-based AsyncClient instead of thread-based Client so that the execution of the script is
    # terminated properly if an assertion fails. With Client, some hanging threads will remain, and they
    # prevent the script from terminating.
    sio: socketio.AsyncClient = socketio.AsyncClient()

    connected: asyncio.Event = asyncio.Event()

    @sio.event
    async def connect():
        connected.set()

    @sio.event()
    def update_parameter(data):
        name: str = data['name']
        value: float = data['value']

        parameters[name] = float(value)

    @sio.event()
    def update_state(data):
        state_variable = data['state_variable']
        value = data['value']

        state[state_variable] = float(value)

    async def send_parameter(name, value):
        data: Dict[str, Any] = {
            'name': name,
            'value': value,
        }
        await sio.emit('update_parameter', data)

    await sio.connect('http://{}:{}'.format(host, port))
    await connected.wait()

    # Test that a parameter update event is received back when sending a parameter.

    await send_parameter('intensity', 123)
    await asyncio.sleep(ROUNDTRIP_MAX_TIME)

    assert parameters == {
        'intensity': 123,
    }

    # Test sending another parameter.

    await send_parameter('iti', 500)
    await asyncio.sleep(ROUNDTRIP_MAX_TIME)

    assert parameters == {
        'intensity': 123,
        'iti': 500,
    }

    await sio.disconnect()
