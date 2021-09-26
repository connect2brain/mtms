#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example
# -------
#
# Initialize by:
#
#     poetry install
#
# Run by:
#
#     poetry run python backend_client.py
#
# Connects to the backend and does the following:
#
# - Prints the parameter values when they are updated.
# - Listens to the state and prints it when it is updated.
# - Allows sending commands to the backend.

import socketio

sio = socketio.Client()
sio.connect('http://localhost:5000')

@sio.event()
def update_parameter(data):
    name = data['name']
    value = data['value']
    print("Value updated for parameter '{}': {}".format(name, value))

@sio.event()
def update_state(data):
    state_variable = data['state_variable']
    value = data['value']
    print("Value updated for state variable '{}': {}".format(state_variable, value))

print('my sid is', sio.sid)

while True:
    splitted = input("").split()
    if len(splitted) == 1:
        command = splitted[0]
        sio.emit('command', command)
    else:
        name, value = splitted
        data = {
            'name': name,
            'value': int(value),
        }
        sio.emit('update_parameter', data)
