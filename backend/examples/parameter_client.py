#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example
# -------
#
# Run by:
#
#     pipenv run python -m backend.examples.parameter_client
#
# Connects to the parameter server in the backend and prints the parameter values when they are updated.

import socketio

sio = socketio.Client()
sio.connect('http://localhost:5000', namespaces=['/parameters'])

@sio.event(namespace='/parameters')
def update_parameter(data):
    name = data['name']
    value = data['value']
    print("Value updated for parameter '{}': {}".format(name, value))

print('my sid is', sio.sid)

while True:
    name, value = input("").split()
    data = {
        'name': name,
        'value': int(value),
    }
    sio.emit('update_parameter', data, namespace='/parameters')
