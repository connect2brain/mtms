#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example
# -------
#
# Run by:
#
#     pipenv run python -m examples.socketio_client
#
# Connects to the parameter server in the backend and prints the parameter values when they are updated.

import socketio

sio = socketio.Client()
sio.connect('http://localhost:5000', namespaces=['/parameters'])

@sio.event(namespace='/parameters')
def update_parameter(data):
    topic = data['topic']
    value = data['value']
    print("Value updated for parameter '{}': {}".format(topic, value))

print('my sid is', sio.sid)

while True:
    topic, value = input("").split()
    data = {
        'topic': topic,
        'value': value,
    }
    sio.emit('update_parameter', data, namespace='/parameters')
