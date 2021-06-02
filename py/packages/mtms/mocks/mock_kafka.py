import os
import time
import queue
from queue import Queue
from threading import Thread
from typing import Any, Callable, Dict, Optional, TypedDict, Union

# TODO: Be more explicit and stringent about the allowed value types:
#       should they always be JSON dicts?
KafkaValue = Union[str, dict]

class KafkaMessage(TypedDict):
    value: KafkaValue

KafkaMessage = Dict[str, Any]

Topic = str

KafkaCallback = Callable[[Topic, KafkaValue], None]


class MockKafkaConsumer:
    def __init__(self, queue: Queue, timeout: float) -> None:
        self.queue: Queue = queue

        # Unused
        self.timeout: float = timeout

    def poll(self) -> Optional[KafkaMessage]:
        msg: Optional[KafkaMessage] = None
        try:
            msg = self.queue.get(block=False)
        except queue.Empty:
            pass

        return msg

    def message_to_value(self, message: KafkaMessage) -> KafkaValue:
        return message["value"]


class MockKafka:
    """A class for mocking communication with Kafka.

    """

    def __init__(self) -> None:
        self.queues: Dict[Topic, Queue] = {}

    def get_consumer(self, topic: Topic, timeout: float = 0) -> MockKafkaConsumer:
        queues: Dict[Topic, Queue] = self.queues

        if topic not in queues:
            queues[topic] = Queue()

        consumer: MockKafkaConsumer = MockKafkaConsumer(
            queue=queues[topic],
            timeout=timeout,
        )
        return consumer

    def produce(self, topic: Topic, value: Any) -> None:
        queues: Dict[Topic, Queue] = self.queues

        # TBD: Does not simulate Confluent Kafka's demand for creating the topics in advance.
        if topic not in queues:
            queues[topic] = Queue()

        self.queues[topic].put({
            "value": value,
        })


class MockKafkaListener(Thread):
    def __init__(self,
            kafka: MockKafka,
            topic: Topic,
            callback: KafkaCallback,
            delay: float = 0.1,
            verbose: bool = True) -> None:

        self.kafka: MockKafka = kafka
        self.topic: Topic = topic
        self.callback: KafkaCallback = callback
        self.consumer: MockKafkaConsumer = self.kafka.get_consumer(topic)

        # Unused
        self.delay: float = delay
        self.verbose: bool = verbose

        Thread.__init__(self)
        self.daemon: bool = True

    def run(self) -> None:
        while True:
            msg: Optional[KafkaMessage] = self.consumer.poll()
            if msg is not None:
                value: KafkaValue = self.consumer.message_to_value(msg)
                self.callback(self.topic, value)
