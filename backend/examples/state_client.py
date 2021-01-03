#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example
# -------
#
# Run by:
#
#     pipenv run python -m backend.examples.command_client
#
# Connects to the state server in the backend and listens to the state.

import socketio

sio = socketio.Client()
sio.connect('http://localhost:5000', namespaces=['/state'])

@sio.event(namespace='/state')
def update_state(data):
    state_variable = data['state_variable']
    value = data['value']
    print("Value updated for state variable '{}': {}".format(state_variable, value))

print('my sid is', sio.sid)

while True:
    pass
