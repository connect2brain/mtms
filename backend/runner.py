#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

import dotenv

from .app import create_app
from .cyclic_buffer import CyclicBuffer
from .eeg_reader import EegReader
from kafka.util import get_kafka_consumer

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

eeg_buffer_length = int(os.getenv("BACKEND_EEG_BUFFER_LENGTH", 8192))
# TODO: Sender should publish these via Kafka.
sampling_frequency = 160
n_channels = 64

# Set up Kafka consumer.
consumer = get_kafka_consumer(reset_offset=True)
if consumer is None:
    sys.stderr.write("[ERROR] Could not initialize Kafka consumer.")
    sys.exit(1)

# Create app
app = create_app()

# Initialize buffer for EEG data, store in app
app.eeg_buffer = CyclicBuffer(eeg_buffer_length, n_channels)

# Set up EEG reader, store in app
app.eeg_reader = EegReader(
    name='eeg_reader',
    consumer=consumer,
    buffer=app.eeg_buffer,
    sampling_frequency=sampling_frequency,
)
app.eeg_reader.start()

if __name__ == '__main__':
    app.run()
