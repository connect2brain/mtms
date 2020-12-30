#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example
# -------
#
# Run by:
#
#     pipenv run python -m backend.examples.command_client
#
# Connects to the command server in the backend and allows sending commands to the backend.

import socketio

sio = socketio.Client()
sio.connect('http://localhost:5000', namespaces=['/commands'])

print('my sid is', sio.sid)

while True:
    command = input("")
    sio.emit('command', command, namespace='/commands')
