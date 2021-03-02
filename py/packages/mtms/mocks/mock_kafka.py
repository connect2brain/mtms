import os
import time
import queue
from threading import Thread

class MockKafka:
    """A class for mocking communication with Kafka.

    """

    def __init__(self):
        self.queues = {}

    def reset_consumer(self, consumer=None):
        pass

    def get_consumer(self, topic=None):
        queues = self.queues

        if topic not in queues:
            queues[topic] = queue.Queue()

        class Consumer:
            def __init__(self):
                self.topic = topic

            def consume(self):
                msg = None
                try:
                    msg = queues[self.topic].get(block=False)
                except queue.Empty:
                    pass

                return msg

        consumer = Consumer()
        return consumer

    def get_producer(self, topic=None):
        queues = self.queues

        if topic not in queues:
            queues[topic] = queue.Queue()

        class Producer:
            def __init__(self):
                self.topic = topic

            def produce(self, msg):
                queues[self.topic].put(msg)

        producer = Producer()
        return producer

    def get_listener(self, topic=None, callback=None):
        queues = self.queues
        consumer = self.get_consumer(topic)

        class KafkaListener(Thread):
            def __init__(self):
                self.topic = topic
                self.callback = callback

                Thread.__init__(self)
                self.daemon = True

            def run(self):
                while True:
                    value = consumer.consume()
                    if value is not None:
                        self.callback(self.topic, value)

        listener = KafkaListener()
        return listener
