import json
import logging
import sys
import time
from threading import Thread
from typing import Any

from pykafka.simpleconsumer import SimpleConsumer
import pykafka.exceptions

from mtms.kafka.kafka import Kafka, KafkaMessageType
from .cyclic_buffer import CyclicBuffer

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# XXX: Unify with KafkaListener, which implements most of the same functionality.
class EegListener(Thread):
    """A listener for EEG data, running in a thread.

    """

    def __init__(self, kafka: Kafka, name: str, topic: str,
                 buffer: CyclicBuffer, sampling_frequency: float) -> None:
        """Initialize the reader.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        name
            The name of the thread.
        topic
            The topic for reading new data.
        buffer
            The buffer into which new data are appended.
        sampling_frequency 
            The sampling frequency for EEG data.
        """
        Thread.__init__(self)
        self.kafka: Kafka = kafka
        self.name: str = name
        self.buffer: CyclicBuffer = buffer
        self.sampling_frequency: float = sampling_frequency
        self.daemon: bool = True
        self.consumer: SimpleConsumer = self.kafka.get_consumer(topic=topic)

    def _read_message(self) -> KafkaMessageType:
        """Read one message from Kafka, return the de-serialized message.

        Notes
        -----
        Returns None if there are no new messages.
        """
        message: KafkaMessageType = None
        try:
            logging.info("Polling Kafka for new messages")
            # TODO: Add type annotation when Kafka library is changed.
            raw_message = self.consumer.consume()
            if raw_message is not None:
                logging.info("Reading a message from Kafka")
                message = json.loads(raw_message.value)
        except pykafka.exceptions.SocketDisconnectedError as e:
            sys.stderr.write("[ERROR] Kafka socket disconnected. Reason: '{}'".format(e))
        except json.decoder.JSONDecodeError as e:
            sys.stderr.write("[ERROR] Error decoding JSON message from Kafka. Reason: '{}'\n".format(e))

        return message

    def run(self) -> None:
        """Read EEG messages from Kafka and append them to the buffer.

        """
        logging.info("Starting " + self.name)
        while True:
            message: KafkaMessageType = self._read_message()
            if message is not None:
                self.buffer.append(message['data'], message['time'])
            else:
                time.sleep(1.0 / self.sampling_frequency)
