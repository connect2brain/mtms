#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from typing import Any, Dict, List

from socketio import AsyncServer

from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener
from mtms.db.topic_db import TopicDb

class StateServer:
    """A server for sending state to clients.

    """

    _STATE_TOPIC_TYPE: str = 'state'
    _UPDATE_STATE_EVENT: str = 'update_state'

    def __init__(self, kafka: Kafka, socketio: AsyncServer, topic_db: TopicDb) -> None:
        """Initialize the state server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            An AsyncServer object to which the event listeners are added.
        topic_db
            A TopicDb object to communicate with the topic database.
        """
        self._kafka: Kafka = kafka
        self._socketio: AsyncServer = socketio
        self._topic_db: TopicDb = topic_db

        self._state_topics: List[str] = self._topic_db.get_topics(type=self._STATE_TOPIC_TYPE)
        self._setup_background_tasks()

    def _setup_background_tasks(self) -> None:
        """Setup the background tasks, namely, a Kafka listener for each topic.

        """
        topic: str
        self.background_tasks: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=topic,
                callback=self._update_state,
            ) for topic in self._state_topics
        ]

    async def _update_state(self, topic: str, value: float) -> None:
        """Broadcast the new state to all connected clients.

        Called when a Kafka listener triggers.

        Parameters
        ----------
        topic
            The topic for the new state.
        value
            The new value.
        """
        data: Dict[str, Any] = {
            'state_variable': topic,
            'value': value,
        }
        await self._socketio.emit(
            event=self._UPDATE_STATE_EVENT,
            data=data
        )
