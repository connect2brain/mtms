#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import pytest
import sys
import time

def test_mock_kafka():
    """Tests MockKafka class.

    """
    from mtms.mocks.mock_kafka import MockKafka

    kafka = MockKafka()

    consumer = kafka.get_consumer(topic='test')
    producer = kafka.get_producer(topic='test')

    # Test consuming when the topic is empty.
    value = consumer.consume()
    assert value is None

    # Test producing and consuming a single message.
    producer.produce(123)
    value = consumer.consume()

    assert value == 123

    # Test consuming again after the message has been consumed.
    value = consumer.consume()
    assert value is None

    # Test the order of two messages.
    producer.produce(124)
    producer.produce(125)

    value = consumer.consume()
    assert value == 124

    value = consumer.consume()
    assert value == 125

    # Test producing into two topics.
    consumer_2 = kafka.get_consumer(topic='test2')
    producer_2 = kafka.get_producer(topic='test2')

    producer.produce(999)
    producer_2.produce('')

    value = consumer.consume()
    assert value == 999

    value_2 = consumer_2.consume()
    assert value_2 == ''

    # Test KafkaListener.
    callback_called = False

    def callback(topic, value):
        assert topic == 'test_listener'
        assert value == 12

        nonlocal callback_called
        callback_called = True

    listener = kafka.get_listener(topic='test_listener', callback=callback)
    listener.start()

    producer = kafka.get_producer(topic='test_listener')
    producer.produce(12)

    time.sleep(1)

    assert callback_called
