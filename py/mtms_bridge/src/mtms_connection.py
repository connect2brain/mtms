#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import asyncio
import logging
import os
import time
from threading import Thread
from multiprocessing.connection import Client, Listener

# TODO: Add type hints to this class.
#
class MTMSConnection:
    """A class for connecting between Python and LabVIEW in the mTMS bridge.

    Both Python and LabVIEW use this class for connecting to each other, LabVIEW via calling the Python code using
    a Python node.

    Python end is the server, and LabVIEW end is the client.
    """
    def __init__(self, port, is_server=False):
        """Initialize one end of the mTMS connection.

        Parameters
        ----------
        port : int
            The port in which to connect.
        is_server : boolean
            True if the connecting end is the server, and False if it is the client.
        """
        self._port = port

        # The server needs to listen to all interfaces (0.0.0.0) due to being run in a container.
        self._host = '0.0.0.0' if is_server else 'localhost'

        self._address = (self._host, self._port)     # family is deduced to be 'AF_INET'
        self._is_server = is_server
        self._connection = None
        self._listener = Listener(self._address, authkey=b'secret password') if is_server else None

    def send(self, msg_type=None, param1=None, param2=None):
        """Send a message to the other end.

        Parameters
        ----------
        msg_type : str
            Message type.
        param1 : any
            The first parameter to be passed.
        param2 : any
            The second parameter to be passed.

        Returns
        -------
        boolean
            True if the connection is open and the message was successfully sent, otherwise False.
        """
        if not self.is_connected():
            return False

        try:
            self._connection.send([msg_type, param1, param2])
            return True

        except (ConnectionAbortedError, ConnectionResetError) as e:
            self._connection_lost()
            return False

    def receive(self, block=False):
        """Receive a message from the other end.

        Parameters
        ----------
        block : boolean
            If True, the function waits for a new message if there are not currently any.
            If False, the function returns an empty message if there are no new messages.

        Returns
        -------
        array
            If there is a new message, return an array of three elements:
            the message type, the first parameter, and the second parameter.

            If there are no new messages or there is no connection, return an array
            consisting of three Nones.
        """
        if not self.is_connected():
            return None, None, None

        try:
            while True:
                if not block and not self._connection.poll():
                    return None, None, None

                msg_type, param1, param2 = self._connection.recv()

                if msg_type != 'keep-alive':
                    break

        except (EOFError, ConnectionAbortedError, ConnectionResetError) as e:
            self._connection_lost()
            return None, None, None

        return msg_type, param1, param2

    def is_connected(self):
        """Return true if the connection is open.

        """
        return self._connection is not None

    def _connection_lost(self):
        """Mark the connection closed.

        """
        self._connection = None
        logging.info("Connection lost.")

    def _send_keep_alive(self):
        """Send an internal keep-alive message to the other end to query the status of the connection.

        """
        if self.is_connected():
            sent = self.send(msg_type='keep-alive')
            if sent:
                logging.info("Still connected...")

    def _connect_if_disconnected(self):
        """If not connected, reconnect to the other end.

        The server side waits until the connection is successfully formed.
        """
        if not self.is_connected():
            try:
                if self._is_server:
                    logging.info("Waiting for client to connect to {}...".format(self._address))
                    self._connection = self._listener.accept()
                    logging.info("Connection accepted from {}".format(self._listener.last_accepted))
                else:
                    logging.info("Attempting to connect to server {}.".format(self._address))
                    self._connection = Client(self._address, authkey=b'secret password')
                    logging.info("Connected!")

            except (ConnectionRefusedError, ConnectionResetError) as e:
                logging.info("Connection failed.")

    async def run(self):
        """Connects to the other end, and creates a thread that periodically queries that the connection
        is still open. If not, attempts to reconnect.

        """
        server_or_client_str = "server" if self._is_server else "client"
        asyncio.current_task().name = "{}_mtms_connection".format(server_or_client_str)

        logging.info("mTMS connection coroutine started.")
        try:
            while True:
                self._send_keep_alive()
                self._connect_if_disconnected()
                await asyncio.sleep(1)

        except asyncio.CancelledError as e:
            logging.info("Cancelled task {}".format(asyncio.current_task().name))
            raise e

        # General exception handling is needed here so that exceptions within asyncio coroutines are logged properly.
        except Exception as e:
            logging.exception(e)

    def keep_connected(self):
        """Functions similarly to 'run' coroutine, but uses threads instead of asyncio.

        XXX: The reason for having a similar functionality implemented both using asyncio and
             threads is that combining asyncio with LabVIEW turned out to be problematic:
             how to make the asyncio coroutine run in the background while LabVIEW is running?

        """
        def keep_connected_thread():
            logging.info("Thread started.")
            while True:
                self._send_keep_alive()
                self._connect_if_disconnected()
                time.sleep(1)

        name = "{}_keep_connected".format("server" if self._is_server else "client")
        connection = Thread(name=name, target=keep_connected_thread)
        connection.daemon = True
        connection.start()

    def close(self):
        """Close an existing connection.

        """
        # XXX: Does not shut down the thread properly.
        if self.is_connected():
            self._connection.close()
            self._connection = None

        if self._listener is not None:
            self._listener.close()

        logging.info("Connection closed.")
