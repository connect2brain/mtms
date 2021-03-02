#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from flask_socketio import emit

class StateServer:
    """A server for sending state to clients.

    """

    _STATE_TOPIC_TYPE = 'state'
    _UPDATE_STATE_EVENT = 'update_state'

    def __init__(self, kafka=None, socketio=None, topic_db=None):
        """Initialize the state server.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        socketio : flask_socketio.SocketIO
            A SocketIO object to which the event listeners are added.
        topic_db : TopicDb
            A TopicDb object to communicate with the topic database.
        """
        self._kafka = kafka
        self._socketio = socketio
        self._topic_db = topic_db

        self._state_topics = self._topic_db.get_topics_by_type(self._STATE_TOPIC_TYPE)
        self._setup_listeners()

    def _setup_listeners(self):
        """Setup up a Kafka listener for each topic.

        """
        self._listeners = [
            self._kafka.get_listener(
                topic=topic,
                callback=self._update_state,
            ) for topic in self._state_topics
        ]

        for listener in self._listeners:
            listener.start()

    def _update_state(self, topic, value):
        """Broadcast the new state to all connected clients.

        Called when a Kafka listener triggers.

        Parameters
        ----------
        topic : str
            The topic for the new state.
        value : number
            The new value.
        """
        data = {
            'state_variable': topic,
            'value': value,
        }
        self._socketio.emit(self._UPDATE_STATE_EVENT, data)
