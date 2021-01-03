#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

import dotenv

from flask import Flask
from flask_cors import CORS
from flask_socketio import SocketIO

from .servers.eeg_server import EegServer
from .servers.command_server import CommandServer
from .servers.parameter_server import ParameterServer
from .servers.state_server import StateServer

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

eeg_buffer_length = int(os.getenv("BACKEND_EEG_BUFFER_LENGTH", 8192))

# Create app
app = Flask(__name__)

# Enable cross-origin resource sharing
#
# NB: This is here so that we are able to make XMLHttpRequests from an
#   independent frontend server during the development phase. Later on,
#   we can re-think how we want the frontend and backend to interact.
CORS(app)

# XXX: async_mode='eventlet' is preferred by flask_socketio package, but it causes
#   emitting from background threads to not work. Therefore, settling for the
#   second-most-preferable option 'gevent'. Monkey-patching eventlet package has been
#   proposed as a workaround, but it seems to break Kafka libraries.
#
#   See https://github.com/miguelgrinberg/python-socketio/issues/99 for details.
#
socketio = SocketIO(app, async_mode='gevent')

# Create server for EEG data
eeg_server = EegServer(
    app=app,
    eeg_buffer_length=eeg_buffer_length,
)

# Create server for parameters
parameter_server = ParameterServer(
    socketio=socketio,
)

# Create server for commands
command_server = CommandServer(
    socketio=socketio,
)

# Create server for state
state_server = StateServer(
    socketio=socketio,
)

# Run app
if __name__ == '__main__':
    socketio.run(app)
