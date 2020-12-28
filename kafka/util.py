import dotenv
import os

import pykafka
from pykafka import KafkaClient

from db.topics import TopicDb

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

def get_kafka_client(ip=None, port=None, zookeeper_hosts=None, use_greenlets=False):
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
        return None

def get_kafka_consumer(client=None, topic=None):
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
    client = client or get_kafka_client()
    if client is None:
        return None

    consumer = client.topics[topic].get_simple_consumer(
        consumer_timeout_ms=1000,
        auto_offset_reset=pykafka.common.OffsetType.LATEST,
        reset_offset_on_start=True,
    )

    # Reset offset to the last message if the topic is latched.
    topic_db = TopicDb()
    if topic_db.is_topic_latched(topic):
        previous_offset = lambda offset : offset - 1 if offset > 0 else pykafka.common.OffsetType.EARLIEST
        offsets = [(p, previous_offset(op.last_offset_consumed))
                    for p, op in consumer._partitions.items()]
        consumer.reset_offsets(offsets)

    return consumer

def get_kafka_producer(client=None, topic=None):
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
    client = client or get_kafka_client()
    if client is None:
        return None

    client = get_kafka_client()

    producer = client.topics[topic].get_producer(
        sync=False,
        delivery_reports=True,
        linger_ms=0,
    )
    return producer
