#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from flask import Flask
from flask_cors import CORS
from flask_socketio import SocketIO

from mtms.kafka.kafka import Kafka
from mtms.db.topic_db import TopicDb

from servers.eeg_server import EegServer
from servers.command_server import CommandServer
from servers.parameter_server import ParameterServer
from servers.planner_server import PlannerServer
from servers.state_server import StateServer

backend_port: int = int(os.getenv("BACKEND_PORT"))
eeg_buffer_length: int = int(os.getenv("BACKEND_EEG_BUFFER_LENGTH", 8192))

# Create connection to Kafka
kafka: Kafka = Kafka()

# Create connection to database
topic_db: TopicDb = TopicDb()

# Create app
app: Flask = Flask(__name__)

# Enable cross-origin resource sharing
#
# NB: This is here so that we are able to make XMLHttpRequests from an
#   independent frontend server during the development phase. Later on,
#   we can re-think how we want the frontend and backend to interact.
CORS(app)
#cors = CORS(app,resources={r"/*":{"origins":"*"}})

# XXX: async_mode='eventlet' is preferred by flask_socketio package, but it causes
#   emitting from background threads to not work. Therefore, settling for the
#   second-most-preferable option 'gevent'. Monkey-patching eventlet package has been
#   proposed as a workaround, but it seems to break Kafka libraries.
#
#   See https://github.com/miguelgrinberg/python-socketio/issues/99 for details.
#
socketio: SocketIO = SocketIO(app, async_mode='gevent', cors_allowed_origins='*')

# Create server for EEG data
eeg_server: EegServer = EegServer(
    kafka=kafka,
    app=app,
    eeg_buffer_length=eeg_buffer_length,
)

# Create server for parameters
parameter_server: ParameterServer = ParameterServer(
    kafka=kafka,
    socketio=socketio,
    topic_db=topic_db,
)

# Create server for commands
command_server: CommandServer = CommandServer(
    kafka=kafka,
    socketio=socketio,
    topic_db=topic_db,
)

# Create server for state
state_server: StateServer = StateServer(
    kafka=kafka,
    socketio=socketio,
    topic_db=topic_db,
)

# Create server for planner
planner_server: PlannerServer = PlannerServer(
    kafka=kafka,
    socketio=socketio
)

# Run app
if __name__ == '__main__':
    socketio.run(app, port=backend_port, host='0.0.0.0')
