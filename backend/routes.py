import os
import sys
import argparse
import json
import json.decoder
import logging
import time
import threading
from threading import Thread

import dotenv
import pykafka.exceptions
from cyclic_buffer import CyclicBuffer
from flask import Flask, request
from pykafka import KafkaClient

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

# TODO: Sender should publish these via Kafka.
sampling_frequency = 160
n_channels = 64

buffer_length = os.getenv("BACKEND_BUFFER_LENGTH", 8192)


logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# Parse command line arguments

parser = argparse.ArgumentParser(description='Backend')
parser.add_argument('--host', dest='host', type=str, nargs='?',
                    default='localhost',
                    help='host machine IP (default: localhost)')
parser.add_argument('--port', dest='port', type=int, nargs='?',
                    default=9092,
                    help='host machine port (default: 9092')
parser.add_argument('--eeg-topic', dest='eeg_topic', type=str, nargs='?',
                    default='eeg_data',
                    help='Kafka topic for EEG data (default: eeg_data')
args = parser.parse_args()

# Set up Kafka client

try:
    client = KafkaClient(hosts=args.host + ':' + str(args.port))
    topic = client.topics[args.eeg_topic]
    consumer = topic.get_simple_consumer(
        consumer_timeout_ms=1000,
#        auto_offset_reset=pykafka.common.OffsetType.LATEST,
#        reset_offset_on_start=True,
    )
except pykafka.exceptions.NoBrokersAvailableError as e:
    sys.stderr.write("[ERROR] Could not initialize Kafka client. Reason: '{}'\n"

                     .format(e))
    sys.exit(1)

buffer = CyclicBuffer(buffer_length, n_channels)

# Create Flask app

app = Flask(__name__)

# TODO: Document the API endpoint
@app.route('/eeg_data')
def get_eeg_data():
    args_from = float(request.args.get('from', -60))
    args_to = float(request.args.get('to', 0))

    t0 = time.time()
    data, timestamps = buffer.get_timerange(t0 + args_from, t0 + args_to)
    timestamps_relative = [t - t0 for t in timestamps]

    result = {
        'data': data.tolist(),
        'timestamps': timestamps_relative,
    }
    return json.dumps(result)

# Set up reading EEG data

class ReadEeg(Thread):
    """A reader for EEG data.

    """
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name
        self.daemon = True

    def _read_message(self):
        """Read one message from Kafka, return the de-serialized message.

        Notes
        -----
        Returns None if there are no new messages.
        """
        message = None
        try:
            raw_message = consumer.consume()
            if raw_message is not None:
                message = json.loads(raw_message.value)
        except pykafka.exceptions.SocketDisconnectedError as e:
            sys.stderr.write("[ERROR] Kafka socket disconnected. Reason: '{}'".format(e))
        except json.decoder.JSONDecodeError as e:
            sys.stderr.write("[ERROR] Error decoding JSON message from Kafka. Reason: '{}'\n".format(e))

        return message

    def run(self, buffer):
        """Read EEG messages from Kafka and append them to the buffer.

        Parameters
        ----------
        buffer : CyclicBuffer
            The buffer into which new data are appended.
        """
        logging.info("Starting " + self.name)
        while True:
            message = self._read_message()
            if message is not None:
                logging.info("Read a message from the queue")
                buffer.append(message['data'], message['time'])
            else:
                time.sleep(1.0 / sampling_frequency)

if __name__ == '__main__':
    eeg_thread = ReadEeg('read_eeg')
    eeg_thread.start(buffer)
    app.run()
