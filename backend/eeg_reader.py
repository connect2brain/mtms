import json
import logging
import time
from threading import Thread

import pykafka.exceptions

from kafka.util import get_kafka_consumer

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class EegReader(Thread):
    """A reader for EEG data, running in a thread.

    """
    def __init__(self, name=None, consumer=None,
                 buffer=None, sampling_frequency=None):
        """Initialize the reader.

        Parameters
        ----------
        name : str
            The name of the thread.
        consumer : KafkaConsumer
            The Kafka consumer used to read new data.
        buffer : CyclicBuffer
            The buffer into which new data are appended.
        sampling_frequency : float
            The sampling frequency for EEG data.
        """
        Thread.__init__(self)
        self.name = name
        self.consumer = consumer
        self.buffer = buffer
        self.sampling_frequency = sampling_frequency
        self.daemon = True

    def _read_message(self):
        """Read one message from Kafka, return the de-serialized message.

        Notes
        -----
        Returns None if there are no new messages.
        """
        message = None
        try:
            logging.info("Polling Kafka for new messages")
            raw_message = self.consumer.consume()
            if raw_message is not None:
                logging.info("Reading a message from Kafka")
                message = json.loads(raw_message.value)
        except pykafka.exceptions.SocketDisconnectedError as e:
            sys.stderr.write("[ERROR] Kafka socket disconnected. Reason: '{}'".format(e))
        except json.decoder.JSONDecodeError as e:
            sys.stderr.write("[ERROR] Error decoding JSON message from Kafka. Reason: '{}'\n".format(e))

        return message

    def run(self):
        """Read EEG messages from Kafka and append them to the buffer.

        """
        logging.info("Starting " + self.name)
        while True:
            message = self._read_message()
            if message is not None:
                self.buffer.append(message['data'], message['time'])
            else:
                time.sleep(1.0 / self.sampling_frequency)
