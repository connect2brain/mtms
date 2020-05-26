import dotenv
import os

import pykafka
from pykafka import KafkaClient

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

def get_kafka_client(ip=None, port=None):
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
        client = KafkaClient(hosts="{ip}:{port}".format(
            ip=(ip or os.getenv("KAFKA_IP") or '127.0.0.1'), 
            port=(port or os.getenv("KAFKA_PORT") or '9092')))
        return client
    except pykafka.exceptions.NoBrokersAvailableError as e:
        return None

def get_kafka_consumer(client=None, topic=None, reset_offset=False):
    """Initializes and returns a KafkaConsumer.

    Parameters
    ----------
    client : KafkaClient
        A KafkaClient object used for fetching the consumer.
    topic : str
        The name of the Kafka topic.
    reset_offset : boolean
        Indicates if the offset should be reset to the beginning of the topic.

    Returns
    -------
    KafkaConsumer or None
        An initialized KafkaConsumer on success, or None in case of failure.
    """
    client = client or get_kafka_client()
    if client is None:
        return None

    topic = client.topics[(topic or os.getenv("KAFKA_TOPIC") or 'eeg_data')]
    consumer = topic.get_simple_consumer(
        consumer_timeout_ms=1000,
        auto_offset_reset=pykafka.common.OffsetType.LATEST,
        reset_offset_on_start=reset_offset,
    )
    return consumer
