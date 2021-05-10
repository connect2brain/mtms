#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import time

from flask import request

from .eeg.cyclic_buffer import CyclicBuffer

class EegServer():
    """A server for EEG data.

    Reads data from a Kafka topic and serves it at /eeg_data endpoint.
    """
    _EEG_TOPIC = 'eeg_data'

    def __init__(self, kafka=None, app=None, eeg_buffer_length=None):
        """Initialize the EEG server.

        Parameters
        ----------
        kafka : Kafka
            A Kafka object to communicate with Kafka.
        app : Flask
            The Flask application that the endpoint is added to.
        eeg_buffer_length : int
            The length of the buffer for EEG data, in samples.
        """
        self._kafka = kafka
        self._eeg_buffer_length = eeg_buffer_length

        # TODO: Sender should publish these via Kafka.
        self._sampling_frequency = 160
        self._n_channels = 64

        self._initialize_eeg_listener()

        # TODO: Document the API endpoint, modify to use Socket.IO.
        @app.route('/eeg_data')
        def get_eeg_data():
            args_from = float(request.args.get('from', -60))
            args_to = float(request.args.get('to', 0))

            t0 = time.time()
            data, timestamps = self._eeg_buffer.get_timerange(
                t0 + args_from,
                t0 + args_to,
            )
            timestamps_relative = [t - t0 for t in timestamps]

            result = [
                {'data': data, 'timestamp': timestamp}
                for data, timestamp in zip(data.tolist(), timestamps_relative)
            ]
            return json.dumps(result), 200, {'content-type': 'application/json'}

    def _initialize_eeg_listener(self):
        """Initialize the EEG listener.

        """
        self._eeg_buffer = CyclicBuffer(
            self._eeg_buffer_length,
            self._n_channels,
        )
        def callback(topic, raw_message):
            message = json.loads(raw_message)
            self._eeg_buffer.append(message['data'], message['time'])

        delay = 1.0 / self._sampling_frequency
        self._eeg_listener = self._kafka.get_listener(
            topic=self._EEG_TOPIC,
            callback=callback,
            delay=delay
        )
        self._eeg_listener.start()
