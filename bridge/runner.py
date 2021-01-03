#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import sys
import time
from queue import Queue

from db.topics import TopicDb
from kafka.util import get_kafka_producer
from .mtms_connection import MtmsConnection
from .topic_queue import TopicQueue
from .activex_listener import ActiveXListener

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# Initialization
mtms = MtmsConnection()
topic_db = TopicDb()
polling_interval = 1

# Communicate from LabVIEW to Kafka
state_topics = topic_db.get_topics_by_type('state')
state_control_names = topic_db.get_control_names_for_topics(state_topics)

activex_listeners = {}
for topic in state_topics:
    producer = get_kafka_producer(topic=topic)
    callback = lambda value, producer=producer: producer.produce(bytes(str(value), encoding='utf8'))

    control_name = state_control_names[topic]
    listener = ActiveXListener(control_name=control_name, callback=callback, polling_interval=polling_interval)

    listener.start()
    activex_listeners[topic] = listener

# Communicate from Kafka to LabVIEW
parameter_topics = topic_db.get_topics_by_type('parameter')
command_topics = topic_db.get_topics_by_type('command')

topics = parameter_topics + command_topics
control_names = topic_db.get_control_names_for_topics(topics)

topic_queue = TopicQueue(topics)

while True:
    data = topic_queue.get()
    topic = data['topic']
    value = data['value']
    if topic in parameter_topics:
        try:
            value = int(value)
        except ValueError as e:
            logging.error("Invalid numeric value in topic '{topic}'. Reason: '{error}'".format(
                topic=topic,
                error=e
            ))
    else:
        value = True

    control_name = control_names[topic]
    mtms.set_value(
        control_name=control_name,
        value=value,
    )
