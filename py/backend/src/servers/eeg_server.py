#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import logging
import time
from typing import Any, Dict, List, Tuple, TypedDict

from socketio import AsyncServer
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

    def __init__(self, kafka: Kafka, socketio: AsyncServer, eeg_buffer_length: int) -> None:
        """Initialize the EEG server.

        Parameters
        ----------
        kafka
            A Kafka object to communicate with Kafka.
        socketio
            An AsyncServer object to which the event listeners are added.
        eeg_buffer_length
            The length of the buffer for EEG data, in samples.
        """
        self._kafka: Kafka = kafka
        self._socketio: AsyncServer = socketio
        self._eeg_buffer_length: int = eeg_buffer_length

        # TODO: Sender should publish these via Kafka.
        self._sampling_frequency: int = 160
        self._n_channels: int = 64

        socketio.on(
            event='eeg_data',
            handler=self._send_eeg_data,
        )

        self._setup_background_tasks()

    def _send_eeg_data(self, client_id: str, params: Dict[str, Any]) -> None:
        """Return EEG data on request.

        Parameters
        ----------
        client_id
            The client id, provided by the AsyncServer.
        params
            A dict consisting of optional 'from' and 'to' keys, with values determining the
            start and the end (in seconds) for the fetched data, relative to the current time.

            'from' defaults to -60 seconds, and 'to' defaults to 0 (the current time).
        """
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
        return result

    def _setup_background_tasks(self) -> None:
        """Setup the background tasks, namely, a task for listening to Kafka topic for new EEG data.

        """
        self._eeg_buffer: CyclicBuffer = CyclicBuffer(
            self._eeg_buffer_length,
            self._n_channels,
        )
        async def callback(topic: str, raw_message: str):
            message: Dict[str, Any] = json.loads(raw_message)
            self._eeg_buffer.append(message['data'], message['time'])

        delay: float = 1.0 / self._sampling_frequency

        self.background_tasks: List[KafkaListener] = [
            KafkaListener(
                kafka=self._kafka,
                topic=self._EEG_TOPIC,
                callback=callback,
                delay=delay,

                # Be less verbose here due to the plentitude of EEG messages.
                verbose=False,
            ),
        ]
