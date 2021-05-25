#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import logging
import time
from typing import List, Tuple, TypedDict

import flask_socketio
from flask_socketio import SocketIO
from numpy.typing import ArrayLike

from mtms.kafka.kafka import Kafka
from mtms.kafka.listener import KafkaListener
from .eeg.cyclic_buffer import CyclicBuffer

class EegDataPoint(TypedDict):
    data: ArrayLike
    timestamp: float

EegData = List[EegDataPoint]

class EegServer():
    """A server for EEG data.

    Reads data from a Kafka topic and serves it at /eeg_data endpoint.
    """
    _EEG_TOPIC: str = 'eeg_data'

    def __init__(self, kafka: Kafka, socketio: SocketIO, eeg_buffer_length: int) -> None:
        """Initialize the EEG server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            A SocketIO object to which the event listeners are added.
        eeg_buffer_length
            The length of the buffer for EEG data, in samples.
        """
        self._kafka: Kafka = kafka
        self._socketio: SocketIO = socketio
        self._eeg_buffer_length: int = eeg_buffer_length

        # TODO: Sender should publish these via Kafka.
        self._sampling_frequency: int = 160
        self._n_channels: int = 64

        self._initialize_eeg_listener()

        socketio.on_event('eeg_data', self._send_eeg_data)

    def _send_eeg_data(self, params) -> None:
        args_from: float = float(params.get('from', -60))
        args_to: float = float(params.get('to', 0))

        t0: float = time.time()

        data: ArrayLike
        timestamps: ArrayLike
        data, timestamps = self._eeg_buffer.get_timerange(
            t0 + args_from,
            t0 + args_to,
        )
        timestamps_relative: ArrayLike = [t - t0 for t in timestamps]

        result: EegData = [
            {'data': data, 'timestamp': timestamp}
            for data, timestamp in zip(data.tolist(), timestamps_relative)
        ]
        flask_socketio.emit('eeg_data', result)

    def _initialize_eeg_listener(self) -> None:
        """Initialize the EEG listener.

        """
        self._eeg_buffer: CyclicBuffer = CyclicBuffer(
            self._eeg_buffer_length,
            self._n_channels,
        )
        def callback(topic, raw_message):
            message = json.loads(raw_message)
            self._eeg_buffer.append(message['data'], message['time'])

        delay = 1.0 / self._sampling_frequency
        self._eeg_listener: KafkaListener = self._kafka.get_listener(
            topic=self._EEG_TOPIC,
            callback=callback,
            delay=delay
        )
        self._eeg_listener.start()
