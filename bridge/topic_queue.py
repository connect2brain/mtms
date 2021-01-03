#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from queue import Queue

from kafka.listener import KafkaListener

class TopicQueue():
    """A helper class that pushes new data from several topics into a single queue.

    """
    def __init__(self, topics):
        """Initialize the topic queue.

        Parameters
        ----------
        topics : array_like
            An array of topic names.
        """
        self._topics = topics
        self._queue = Queue()

        callback = lambda topic, value:\
            self._queue.put({
                'topic': topic,
                'value': value
            })

        # Initialize listener for each topic.
        self._listeners = [
            KafkaListener(
                topic=topic,
                callback=callback,
            ) for topic in topics
        ]

        # Start listener threads.
        for listener in self._listeners:
            listener.start()

    def get(self):
        """Pop from the queue.

        Returns
        -------
        dict
            A dictionary of the form
            {
                'topic': topic,
                'value: value
            }
            where topic is a string indicating the name of the topic, and value is the new value.
        """
        return self._queue.get()