#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import os
import time
from pathlib import Path

import dotenv

from mtms_connection import MTMSConnection

# XXX: Workaround for dotenv to find .env when running the script in LabVIEW.
# See https://github.com/theskumar/python-dotenv/issues/299
dir_path = os.path.dirname(os.path.realpath(__file__))
env_path = Path(dir_path) / '..' / '..' / '..' / '.env'
dotenv.load_dotenv(env_path)

log_directory = os.getenv("LOG_DIRECTORY")
log_filename = Path(log_directory) / 'mtms_client.log'

logging.basicConfig(filename=log_filename,
                    level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s', )

client = MTMSConnection(is_server=False)


def send_state(state_variable, value):
    """Send state to the server.

    Parameters
    ----------
    state_variable : str
        The name of the state variable.
    value : number
        The new value of the state variable.

    Returns
    -------
    number
        1 if the state was successfully sent, otherwise 0.
    """
    state_str = '{} = {}'.format(state_variable, str(value))
    logging.info('Sending state: {}'.format(state_str))

    sent = client.send(msg_type='state', param1=state_variable, param2=value)
    if sent:
        logging.info('State sent: {}'.format(state_str))
    return 1 if sent else 0


def connect():
    """Connect to the server and ensure that the connection is retained.

    """
    client.keep_connected()
    while not client.is_connected():
        time.sleep(1)


def read_message():
    """Read a message from the server and return it to LabVIEW.

    Returns
    -------
    array
        An array consisting of four values:

        [0] 'True' if there was a new message, otherwise 'False'.

        [1] The message type, either 'parameter' or 'command'.
            A dummy string '' if there are no new messages.

        [2] The parameter name or the command, depending on the message type.
            A dummy string '' if there are no new messages.

        [3] The parameter value if the message type is 'parameter'.
            A dummy string '' if there are no new messages or the message type is 'command'.
    """
    msg_type, param1, param2 = client.receive()

    if msg_type is None:
        return ['False', '', '', '']

    elif msg_type == 'parameter':
        parameter = param1
        value = param2

        logging.info("[Done] Receive parameter: {} = {}".format(parameter, value))
        return ['True', 'parameter', parameter, str(value)]

    elif msg_type == 'command':
        command = param1

        logging.info("[Done] Receive command: {}".format(command))
        return ['True', 'command', command, '']

    else:
        logging.error("[Done] Received a message of the unknown type {}".format(msg_type))
