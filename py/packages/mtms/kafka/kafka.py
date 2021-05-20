import logging
import os
import time
import socket

from confluent_kafka.admin import AdminClient
from confluent_kafka import Consumer, Producer

from mtms.db.topic_db import TopicDb
from .listener import KafkaListener

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

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
        self.zookeeper_hosts = zookeeper_hosts or os.getenv("ZOOKEEPER_HOSTS")

        self.hosts = "{ip}:{port}".format(
            ip=(ip or os.getenv("KAFKA_IP") or '127.0.0.1'),
            port=(port or os.getenv("KAFKA_PORT") or '9092')
        )

        # Initialize producer
        conf = {
            'bootstrap.servers': self.hosts,
            'client.id': socket.gethostname(),
        }

        logging.info("[INFO] Connecting to Kafka broker at {}.\n".format(self.hosts))
        self.producer = Producer(conf)

    def reset_consumer(self, consumer=None):
        """Reset consumer to read the last message in the topic. If the topic is latched, reset consumer to
        re-read the second-last message.

        Parameters
        ----------
        consumer : KafkaConsumer
            The consumer to reset.
        """
        # TBD: This was removed when Kafka library was changed. Rewrite an implementation
        #      for this and add test, as the tests don't seem to break even if this function
        #      doesn't exist.
        pass

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
        conf = {
            'bootstrap.servers': self.hosts,
            'group.id': "group",
        }
        consumer = Consumer(conf)
        consumer.subscribe([topic])

        self.reset_consumer(consumer=consumer)
        return consumer

    def produce(self, *args, **kwargs):
        """Produce a message to a given topic.

        Parameters
        ----------
        topic : str
            The name of the Kafka topic.

        """
        self.producer.produce(*args, **kwargs)

    def flush(self):
        """Flush awaiting events.

        """
        self.producer.flush()

    def create_admin_client(self):
        """Create and return an admin client for Kafka.

        """
        admin_client: AdminClient = AdminClient({
            'bootstrap.servers': self.hosts
        })
        return admin_client

    def get_listener(self, topic=None, callback=None, delay=0.1):
        """Initializes and returns a KafkaListener.

        Parameters
        ----------
        topic : str
            The name of the Kafka topic.
        callback : callable
            The function that is called when a new message is received.
        delay : float
            The delay (in seconds) between two consecutive runs of the listener.
            Defaults to 0.1 seconds.

        Returns
        -------
        KafkaListener
            An initialized KafkaListener.
        """
        listener = KafkaListener(
            kafka=self,
            topic=topic,
            callback=callback,
            delay=delay,
        )
        return listener
