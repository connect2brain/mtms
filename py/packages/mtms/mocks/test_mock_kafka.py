#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import pytest
import sys
import time

def test_mock_kafka():
    """Tests MockKafka class.

    """
    from mtms.mocks.mock_kafka import MockKafka, MockKafkaListener

    kafka = MockKafka()

    consumer = kafka.get_consumer(topic='test')

    # Test consuming when the topic is empty.
    value = consumer.poll()
    assert value is None

    # Test producing and consuming a single message.
    kafka.produce(
        topic='test',
        value=123,
    )
    message = consumer.poll()
    value = consumer.message_to_value(message)

    assert value == 123

    # Test consuming again after the message has been consumed.
    message = consumer.poll()
    assert message is None

    # Test the order of two messages.
    kafka.produce(
        topic='test',
        value=124,
    )
    kafka.produce(
        topic='test',
        value=125,
    )

    message = consumer.poll()
    value = consumer.message_to_value(message)
    assert value == 124

    message = consumer.poll()
    value = consumer.message_to_value(message)
    assert value == 125

    # Test producing into two topics.
    consumer_2 = kafka.get_consumer(topic='test2')
    kafka.produce(
        topic='test',
        value=999,
    )
    kafka.produce(
        topic='test2',
        value='',
    )

    message = consumer.poll()
    value = consumer.message_to_value(message)
    assert value == 999

    message_2 = consumer_2.poll()
    value_2 = consumer.message_to_value(message_2)
    assert value_2 == ''

    # Test KafkaListener.
    callback_called = False

    def callback(topic, value):
        assert topic == 'test_listener'
        assert value == 12

        nonlocal callback_called
        callback_called = True

    listener = MockKafkaListener(
        kafka=kafka,
        topic='test_listener',
        callback=callback,
    )
    listener.start()

    kafka.produce(
        topic='test_listener',
        value=12,
    )

    time.sleep(1)

    assert callback_called
