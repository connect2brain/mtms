#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import os
import sys

from typing import List

from mtms.kafka.kafka import Kafka
from mtms.db.topic_db import TopicDb

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# Create connection to Kafka
kafka: Kafka = Kafka()

# Create connection to database
topic_db: TopicDb = TopicDb()

# Create Kafka topics
topics: List[str] = topic_db.get_topics()

logging.info("Attempting to create topics: {}".format(", ".join(topics)))

kafka.create_topics(topics)

logging.info("Done.")
