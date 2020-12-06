#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import os
import sys
import time
from queue import Queue

import dotenv

import db.topics as topics
from .mtms_connection import MtmsConnection
from kafka.topic_queue import TopicQueue


logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

dotenv.load_dotenv()   # Load configuration from env vars and .env -file

app_name = os.getenv('MTMS_APP_NAME', "MTMSActiveXServer")
app_location = os.getenv('MTMS_APP_LOCATION')
app_filename = os.getenv('MTMS_APP_FILENAME')
vi_name = os.getenv('MTMS_VI_NAME', "mTMS ActiveX Server.vi")

# Initialize ActiveX connection
mtms = MtmsConnection(
    app_name=app_name,
    app_location=app_location,
    app_filename=app_filename,
    vi_name=vi_name,
)

# Create queue for new data in parameter-containing topics
topics, control_names = topics.get_parameter_topics()

topic_queue = TopicQueue(topics)

while True:
    data = topic_queue.get()
    topic = data['topic']
    value = data['value']
    try:
        value = int(value)
        control_name = control_names[topic]
        mtms.set_value(
            control_name=control_name,
            value=value,
        )
    except ValueError as e:
        logging.error("Invalid numeric value in topic '{topic}'. Reason: '{error}'".format(
            topic=topic,
            error=e
        ))
