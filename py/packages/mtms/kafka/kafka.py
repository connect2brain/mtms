import logging
import os
import time

import pykafka
from pykafka import KafkaClient

from mtms.db.topic_db import TopicDb
from .listener import KafkaListener

class Kafka:
    """A class for communicating with Kafka.

    """

    def __init__(self, ip=None, port=None, zookeeper_hosts=None, use_greenlets=False):
        """Initialize the object.

        Parameters
        ----------
        ip : str
            A valid IP address of the Kafka server.
        port : str or int
            The port of the Kafka server.
        """
        self.ip = ip or os.getenv("KAFKA_IP") or '127.0.0.1'
        self.port = port or os.getenv("KAFKA_PORT") or '9092'
        self.zookeeper_hosts = zookeeper_hosts or os.getenv("ZOOKEEPER_HOSTS")

        self.client = None
        while self.client is None:
            try:
                self.client = KafkaClient(
                    hosts="{ip}:{port}".format(
                        ip=self.ip,
                        port=self.port,
                    ),
                    zookeeper_hosts=(zookeeper_hosts or os.getenv("ZOOKEEPER_HOSTS")),
                    use_greenlets=use_greenlets
                )
            except pykafka.exceptions.NoBrokersAvailableError as e:
                logging.info("Kafka is not available, retrying in 1 s...")
                time.sleep(1)

    def reset_consumer(self, consumer=None):
        """Reset consumer to read the last message in the topic. If the topic is latched, reset consumer to
        re-read the second-last message.

        Parameters
        ----------
        consumer : KafkaConsumer
            The consumer to reset.
        """
        # Reset to the latest message.
        offsets = [(p, pykafka.common.OffsetType.LATEST)
                   for p, op in consumer._partitions.items()]
        consumer.reset_offsets(offsets)

        # Move backwards by one message if the topic is latched.
        topic = consumer.topic.name.decode('utf-8')
        topic_db = TopicDb()
        is_latched = topic_db.is_topic_latched(topic)

        if is_latched:
            previous_offset = lambda offset: offset - 1 if offset > 0 else pykafka.common.OffsetType.EARLIEST
            offsets = [(p, previous_offset(op.last_offset_consumed))
                       for p, op in consumer._partitions.items()]
            consumer.reset_offsets(offsets)

    def get_consumer(self, topic=None):
        """Initializes and returns a KafkaConsumer.

        Parameters
        ----------
        topic : str
            The name of the Kafka topic.

        Returns
        -------
        KafkaConsumer
            An initialized KafkaConsumer.
        """
        consumer = self.client.topics[topic].get_simple_consumer(
            consumer_timeout_ms=1000,
        )
        self.reset_consumer(consumer=consumer)
        return consumer

    def get_producer(self, topic=None):
        """Initializes and returns a KafkaProducer.

        Parameters
        ----------
        topic : str
            The name of the Kafka topic.

        Returns
        -------
        KafkaProducer
            An initialized KafkaProducer.
        """
        producer = self.client.topics[topic].get_producer(
            sync=False,
            delivery_reports=True,
            linger_ms=0,
        )
        return producer

    def get_listener(self, topic=None, callback=None):
        """Initializes and returns a KafkaListener.

        Parameters
        ----------
        topic : str
            The name of the Kafka topic.

        Returns
        -------
        KafkaListener
            An initialized KafkaListener.
        """
        listener = KafkaListener(
            kafka=self,
            topic=topic,
            callback=callback,
        )
        return listener
