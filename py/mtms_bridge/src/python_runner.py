#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import time
from threading import Thread

from mtms.db.topic_db import TopicDb
from mtms.kafka.kafka import Kafka

from mtms_connection import MTMSConnection
from state_receiver import StateReceiver
from parameter_command_sender import ParameterCommandSender

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

# Initialization
kafka = Kafka()
topic_db = TopicDb()
server = MTMSConnection(is_server=True)

state_receiver = StateReceiver(kafka=kafka, server=server, topic_db=topic_db)
parameter_command_sender = ParameterCommandSender(kafka=kafka, server=server, topic_db=topic_db)

server.keep_connected()
state_receiver.start()
parameter_command_sender.start()

while True:
    time.sleep(1)
