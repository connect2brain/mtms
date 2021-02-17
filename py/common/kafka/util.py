import logging
import os
import time

import pykafka
from pykafka import KafkaClient

from py.common.db.topics import TopicDb

def get_kafka_client(ip=None, port=None, block=True, zookeeper_hosts=None, use_greenlets=False):
    """Initializes and returns a KafkaClient.

    Parameters
    ----------
    ip : str
        A valid IP address of the Kafka server.
    port : str or int
        The port of the Kafka server.

    Returns
    -------
    KafkaClient or None
        An initialized KafkaClient on success, or None in case of failure.
    """
    while True:
        try:
            client = KafkaClient(
                hosts="{ip}:{port}".format(
                    ip=(ip or os.getenv("KAFKA_IP") or '127.0.0.1'),
                    port=(port or os.getenv("KAFKA_PORT") or '9092')
                ),
                zookeeper_hosts=(zookeeper_hosts or os.getenv("ZOOKEEPER_HOSTS")),
                use_greenlets=use_greenlets)
            return client
        except pykafka.exceptions.NoBrokersAvailableError as e:
            if not block:
                return None
        logging.info("Kafka is not available, retrying in 1 s...")
        time.sleep(1)

def reset_consumer(consumer):
    # Reset consumer's offset to the last message.
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

def get_kafka_consumer(client=None, topic=None, block=True):
    """Initializes and returns a KafkaConsumer.

    Parameters
    ----------
    client : KafkaClient
        A KafkaClient object used for fetching the consumer.
    topic : str
        The name of the Kafka topic.

    Returns
    -------
    KafkaConsumer or None
        An initialized KafkaConsumer on success, or None in case of failure.
    """
    client = client or get_kafka_client(block=block)
    if client is None:
        return None

    consumer = client.topics[topic].get_simple_consumer(
        consumer_timeout_ms=1000,
    )
    reset_consumer(consumer)
    return consumer

def get_kafka_producer(client=None, topic=None, block=True):
    """Initializes and returns a KafkaProducer.

    Parameters
    ----------
    client : KafkaClient
        A KafkaClient object used for fetching the consumer.
    topic : str
        The name of the Kafka topic.

    Returns
    -------
    KafkaProducer or None
        An initialized KafkaProducer on success, or None in case of failure.
    """
    client = client or get_kafka_client(block=block)
    if client is None:
        return None

    producer = client.topics[topic].get_producer(
        sync=False,
        delivery_reports=True,
        linger_ms=0,
    )
    return producer
