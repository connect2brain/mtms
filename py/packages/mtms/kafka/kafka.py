# TODO: There are still type hints missing in this file, fill them in.

import logging
import os
import time
from typing import Any, Callable, Dict, List, Optional, Union

# Imports for confluent-kafka
import socket
import confluent_kafka
import confluent_kafka.admin

# Imports for pykafka
import pykafka

from mtms.db.topic_db import TopicDb
from .listener import KafkaListener

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)


# TODO: More specific types could be used here. Not sure if KafkaMessage can be more specific,
#       as it needs to conform to both confluent-kafka and pykafka. However, KafkaValue could
#       be more specific: e.g., do we even want to allow other values than JSON-string?

# The message received from Kafka, including metadata.
KafkaMessage = Any

# The content of the message.
KafkaValue = Any

KafkaTopic = str
ProduceError = Optional[str]
ProduceCallback = Callable[[ProduceError, KafkaMessage], None]
Producer = Union[confluent_kafka.Producer, pykafka.producer.Producer]
Consumer = Union[confluent_kafka.Consumer, pykafka.simpleconsumer.SimpleConsumer]

class KafkaConsumer:
    """A class for consuming Kafka messages.

    """

    def __init__(self,
            consumer: Consumer,
            kafka_library: str,
            timeout: float) -> None:
        """Initialize the Kafka consumer.

        Parameters
        ----------
        consumer
            The Kafka library's own Kafka Consumer object.
        kafka_library
            The Kafka library used.
        timeout
            The timeout in seconds when polling for a new message.
        """
        assert kafka_library in ["confluent-kafka", "pykafka"], \
            "Unknown Kafka library: {}".format(kafka_library)

        self._consumer: Consumer = consumer
        self._kafka_library: str = kafka_library
        self._timeout: float = timeout

    def poll(self) -> Optional[KafkaMessage]:
        """Poll one new message from Kafka. Return None if there are no new messages.

        """
        msg: Optional[KafkaMessage]
        if self._kafka_library == "confluent-kafka":
            msg = self._consumer.poll(timeout=self._timeout)

        elif self._kafka_library == "pykafka":
            # In pykafka, a timeout of zero is a special case that needs to be handled using 'block' keyword
            # argument while consuming a message, and not consumer_timeout_ms parameter when creating the consumer;
            # when consumer_timeout_ms is set to 0 in pykafka, it actually causes no timeout to be used at all.
            block: bool = self._timeout != 0
            msg = self._consumer.consume(block=block)

        else:
            assert False

        return msg

    def message_to_value(self, message: KafkaMessage) -> KafkaValue:
        """Given a message returned by poll function, return the content of the message.

        Parameters
        ----------
        message
            The message of interest.
        """
        value: KafkaValue
        if self._kafka_library == "confluent-kafka":
            value = message.value()

        elif self._kafka_library == "pykafka":
            value = message.value

        else:
            assert False

        return value

    def reset(self) -> None:
        """Reset consumer to read the last message in the topic. If the topic is latched, reset consumer to
        re-read the second-last message.

        """
        # TODO: This was removed when support for confluent-kafka was added. Rewrite an implementation
        #       for this and add test, as the tests don't seem to break even if this function
        #       doesn't exist. It's also not needed by any current functionality, so removing it
        #       for now does not break anything.
        pass


class Kafka:
    """A class for communicating with Kafka.

    """

    def __init__(self) -> None:
        """Initialize the Kafka object.

        """
        self.ip = os.getenv("KAFKA_IP")
        self.port = os.getenv("KAFKA_PORT")
        self.zookeeper_hosts = os.getenv("ZOOKEEPER_HOSTS")
        self.kafka_library = os.getenv("KAFKA_LIBRARY")

        self.hosts = "{ip}:{port}".format(
            ip=self.ip,
            port=self.port,
        )

        logging.info("Using the Kafka library '{}'.".format(self.kafka_library))
        logging.info("Connecting to Kafka broker at {}.".format(self.hosts))

        if self.kafka_library == "pykafka":
            self.client = pykafka.KafkaClient(
                hosts=self.hosts,
                zookeeper_hosts=self.zookeeper_hosts,
            )
            self.producers: Dict[KafkaTopic, Producer] = {}

        elif self.kafka_library == "confluent-kafka":
            self.producer = confluent_kafka.Producer({
                'bootstrap.servers': self.hosts,
                'client.id': socket.gethostname(),
            })

        else:
            assert False, \
                "Unknown Kafka library: {}".format(self.kafka_library)

    def get_consumer(self, topic: str = None, timeout: float = 0):
        """Initialize and return a KafkaConsumer.

        Parameters
        ----------
        topic
            The name of the Kafka topic.
        timeout
            The timeout (in seconds) when polling for a new message. Defaults to 0, that is, timeouts instantly
            if there are no new messages.

        Returns
        -------
        KafkaConsumer
            An initialized KafkaConsumer.
        """
        if self.kafka_library == "pykafka":
            timeout_ms: float = 1000 * timeout
            consumer = self.client.topics[topic].get_simple_consumer(
                consumer_group="group",
                consumer_timeout_ms=timeout_ms,
                auto_offset_reset=pykafka.common.OffsetType.LATEST,
                reset_offset_on_start=True,
            )

        elif self.kafka_library == "confluent-kafka":
            consumer = confluent_kafka.Consumer({
                'bootstrap.servers': self.hosts,
                'group.id': "group",
            })
            consumer.subscribe([topic])

        else:
            assert False

        return KafkaConsumer(
            consumer=consumer,
            kafka_library=self.kafka_library,
            timeout=timeout,
        )

    def produce(self,
            topic: str,
            value: KafkaMessage,
            callback: Optional[ProduceCallback] = None) -> None:
        """Produce a message to a given topic.

        Parameters
        ----------
        topic
            The name of the Kafka topic.
        value
            The message produced to the topic.
        callback
            Optional function to be called once the message has been produced. The callback
            takes two positional arguments: the error message and the value that was to be
            produced. The error message is None if the message was produced successfully.
        """
        if self.kafka_library == "pykafka":
            if topic not in self.producers:
                self.producers[topic] = self.client.topics[topic].get_producer(
                    sync=False,
                    delivery_reports=True,
                    linger_ms=0,
                )

            producer = self.producers[topic]
            producer.produce(value)

            if callback is not None:
                # TODO: Propagate potential errors to the callback function, similarly to confluent-kafka.
                err: ProduceError = None
                callback(err, value)

        elif self.kafka_library == "confluent-kafka":
            self.producer.produce(
                topic=topic,
                value=value,
                callback=callback,
            )
            self.producer.flush()

        else:
            assert False

    def create_topics(self, topics: List[str]):
        """Create the given Kafka topics.

        Parameters
        ----------
        topics
            A list of the Kafka topics to be created.
        """
        if self.kafka_library == "confluent-kafka":
            # The code below is adapted from:
            #
            # https://github.com/confluentinc/confluent-kafka-python/blob/master/examples/adminapi.py

            admin_client: confluent_kafka.admin.AdminClient = confluent_kafka.admin.AdminClient({
                'bootstrap.servers': self.hosts
            })

            # TODO: Re-consider the values of num_partitions and replication_factor parameters.
            new_topics: List[confluent_kafka.admin.NewTopic] = [
                confluent_kafka.admin.NewTopic(topic, num_partitions=1, replication_factor=1) for topic in topics
            ]
            fs = admin_client.create_topics(new_topics)

            topic: str
            for topic, f in fs.items():
                try:
                    f.result()
                    logging.info("Topic created: '{}'".format(topic))
                except Exception as e:
                    logging.warning("Failed to create topic '{}': {}".format(topic, e))

        elif self.kafka_library == "pykafka":
            # Not supported by pykafka. However, pykafka does not require creating the topics in
            # advance, either, so it is fine to skip this step.

            logging.info("Using pykafka, therefore not creating the topics.")
            pass

        else:
            assert False
