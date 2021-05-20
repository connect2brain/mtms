import os
import time
import queue
from queue import Queue
from threading import Thread
from typing import Any, Callable, Optional, Union

# TODO: Be more explicit and stringent about the allowed message types:
#       should they always be JSON dicts?
KafkaMessage = Union[str, dict]

Topic = str

KafkaCallback = Callable[[Topic, KafkaMessage], None]

class Consumer:
    def __init__(self, queue: Queue) -> None:
        self.queue: Queue = queue

    def consume(self) -> Optional[KafkaMessage]:
        msg: Optional[KafkaMessage] = None
        try:
            msg = self.queue.get(block=False)
        except queue.Empty:
            pass

        return msg

class KafkaListener(Thread):
    def __init__(self, topic: Topic, callback: KafkaCallback, consumer: Consumer) -> None:
        self.topic: Topic = topic
        self.callback: KafkaCallback = callback
        self.consumer: Consumer = consumer

        Thread.__init__(self)
        self.daemon: bool = True

    def run(self) -> None:
        while True:
            msg: Optional[KafkaMessage] = self.consumer.consume()
            if msg is not None:
                self.callback(self.topic, msg)

class MockKafka:
    """A class for mocking communication with Kafka.

    """

    def __init__(self) -> None:
        self.queues: Dict[Topic, Queue] = {}

    def reset_consumer(self, consumer: Consumer) -> None:
        pass

    def get_consumer(self, topic: Topic) -> Consumer:
        queues: Dict[Topic, Queue] = self.queues

        if topic not in queues:
            queues[topic] = Queue()

        consumer: Consumer = Consumer(queue=queues[topic])
        return consumer

    def produce(self, topic: Topic, value: Any) -> None:
        queues: Dict[Topic, Queue] = self.queues

        # TBD: Does not simulate Confluent Kafka's demand for creating the topics in advance.
        if topic not in queues:
            queues[topic] = Queue()

        self.queues[topic].put(value)

    def get_listener(self, topic: Topic, callback: KafkaCallback) -> KafkaListener:
        consumer: Consumer = self.get_consumer(topic)
        listener: kafkaListener = KafkaListener(
            topic=topic,
            callback=callback,
            consumer=consumer
        )
        return listener
