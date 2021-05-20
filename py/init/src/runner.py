#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import os
import sys

from typing import List

from mtms.kafka.kafka import Kafka
from mtms.db.topic_db import TopicDb

from confluent_kafka import KafkaException
from confluent_kafka.admin import AdminClient, NewTopic

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# Create connection to Kafka
kafka: Kafka = Kafka()

# Create connection to database
topic_db: TopicDb = TopicDb()

# Create Kafka topics
topics: List[str] = topic_db.get_topics()

logging.info("Attempting to create topics: {}".format(", ".join(topics)))

# The code below is adapted from:
#
# https://github.com/confluentinc/confluent-kafka-python/blob/master/examples/adminapi.py

admin_client: AdminClient = kafka.create_admin_client()

# TODO: Re-consider the values of num_partitions and replication_factor parameters.
new_topics: List[NewTopic] = [
    NewTopic(topic, num_partitions=1, replication_factor=1) for topic in topics
]
fs = admin_client.create_topics(new_topics)

for topic, f in fs.items():
    try:
        f.result()
        logging.info("Topic created: '{}'".format(topic))
    except Exception as e:
        logging.warning("Failed to create topic '{}': {}".format(topic, e))

logging.info("Done.")
