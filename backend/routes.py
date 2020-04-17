import sys
import argparse
import json
import json.decoder
from flask import Flask
from pykafka import KafkaClient
import pykafka.exceptions

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
    consumer = topic.get_simple_consumer(consumer_timeout_ms=1000)
except pykafka.exceptions.NoBrokersAvailableError as e:
    sys.stderr.write("[ERROR] Could not initialize Kafka client. Reason: '{}'\n".format(e))
    sys.exit(1)

# Create Flask app

app = Flask(__name__)

@app.route('/eeg_data')
def get_eeg_data():
    # TODO: Serves one message at the time, should rather keep a ring buffer
    #   or such, and serve the contents of the buffer.
    try:
        message = consumer.consume()
        return message.value
    except pykafka.exceptions.SocketDisconnectedError as e:
        sys.stderr.write("[ERROR] Kafka socket disconnected. Reason: '{}'".format(e))
        return {'error': "Socket disconnected. Reason: '{}'".format(e)}, 500
    except json.decoder.JSONDecodeError as e:
        sys.stderr.write("[ERROR] Error decoding JSON message from Kafka. Reason: '{}'\n".format(e))
        return {'error': "Error decoding JSON message from Kafka. Reason: '{}'".format(e)}, 500
    except AttributeError as e:
        sys.stderr.write("[ERROR] Error parsing data from Kafka (no data?). Reason: '{}'".format(e))
        return {'error': "No data available"}, 500

if __name__ == '__main__':
    app.run()
