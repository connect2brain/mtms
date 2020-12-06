#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

import dotenv
from flask import Flask
from flask_cors import CORS

from .app import create_app
from .eeg.server import EegServer

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

# Create server for EEG data
eeg_server = EegServer(
    app=app,
    eeg_buffer_length=eeg_buffer_length,
)

# Run app
if __name__ == '__main__':
    app.run()
