#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import sys
import time
from threading import Thread

import pythoncom

from .mtms_connection import MtmsConnection

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class ActiveXListener(Thread):
    """Pushes the data produced into the given ActiveX control to a callback.

    """
    def __init__(self, control_name=None, callback=None, polling_interval=None):
        """Initialize the listener.

        Parameters
        ----------
        callback : function
            The function that is called when new data are produced in the topic.
        """
        Thread.__init__(self)
        self._control_name = control_name
        self._callback = callback
        self._polling_interval = polling_interval
        self._thread_name = 'activex_listener_' + control_name

        self.daemon = True

    def run(self):
        """Read messages from Kafka and call the callback function with the new data.

        """
        # XXX(okahilak): Due to properties of ActiveX communication and threading, CoInitialize needs to be
        #   called before the ActiveX connection is created for the connection to work.
        pythoncom.CoInitialize()
        self._mtms = MtmsConnection()

        logging.info("Starting thread " + self._thread_name)
        while True:
            value = self._mtms.get_value(
                control_name=self._control_name,
            )
            self._callback(value)
            time.sleep(self._polling_interval)
