import os

import dotenv
from flask import Flask

from cyclic_buffer import CyclicBuffer
from eeg_reader import EegReader
from kafka_tools import get_kafka_consumer

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

eeg_buffer_length = int(os.getenv("BACKEND_EEG_BUFFER_LENGTH", 8192))

# TODO: Sender should publish these via Kafka.
sampling_frequency = 160
n_channels = 64

# Set up Kafka consumer.
consumer = get_kafka_consumer()
if consumer is None:
    sys.stderr.write("[ERROR] Could not initialize Kafka consumer. Reason: '{}'\n"
                     .format(e))
    sys.exit(1)

# Create Flask app
app = Flask(__name__)

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

# Import views
from . import views
