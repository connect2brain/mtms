import dotenv
import os

from confluent_kafka import Consumer

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

def get_kafka_consumer(ip=None, port=None, topic=None, auto_offset_reset='latest'):
    """Initializes and returns a Kafka Consumer with a subscription to a topic.

    Parameters
    ----------
    ip : str
        A valid IP address of the Kafka server.
    port : str or int
        The port of the Kafka server.
    topic : str
        The name of the Kafka topic to subscribe to.
    auto_offset_reset : str
        The value of auto.offset.reset parameter for the Kafka Consumer.
        Defaults to 'latest', resetting the offset to the beginning of the
        topic.

    Returns
    -------
    Consumer or None
        An initialized Consumer on success, or None in case of failure.
    """
    hosts = "{ip}:{port}".format(
        ip=(ip or os.getenv("KAFKA_IP") or '127.0.0.1'),
        port=(port or os.getenv("KAFKA_PORT") or '9092'))

    conf = {'bootstrap.servers': hosts,
            'group.id': "group",
            'auto.offset.reset': auto_offset_reset}
    consumer = Consumer(conf)
    consumer.subscribe([topic or os.getenv("KAFKA_TOPIC") or 'eeg_data'])
    return consumer
