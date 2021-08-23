#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example
# -------
#
# Setup by:
#
#     pip install python-dotenv
#
# Run by:
#
#     python labview_server.py
#
# Runs a Python-end server for Python-LabVIEW bridge and does the following:
#
# - Sends commands "stimulate", "abort", and "recharge".
# - Sets random values for the stimulation parameters "intensity" and "iti".

import asyncio
import os
import logging
import random
import sys
from typing import Dict, List

import dotenv

sys.path.append('../py/mtms_bridge/src')
from mtms_connection import MTMSConnection

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

dotenv.load_dotenv()

PARAMS_TO_SEND: Dict[str, int] = {
    'intensity': random.randint(100, 200),
    'iti': random.randint(200, 300),
}

COMMANDS_TO_SEND: List[str] = {
    'recharge',
    'stimulate',
    'abort',
}

port: int = int(os.getenv("MTMS_BRIDGE_PORT"))

server: MTMSConnection = MTMSConnection(port=port, is_server=True)

async def main() -> None:
    asyncio.create_task(server.run())
    while not server.is_connected():
        await asyncio.sleep(1)

    # Send parameters
    parameter: str
    value: int
    for parameter, value in PARAMS_TO_SEND.items():
        logging.info("Setting the parameter '{}' to value {}".format(parameter, value))

        sent: bool = server.send(msg_type='parameter', param1=parameter, param2=value)
        if sent:
            logging.info("Done")
        else:
            logging.error("Fail")

    # Send commands
    command: str
    for command in COMMANDS_TO_SEND:
        logging.info("Sending the command '{}'".format(command))

        sent: bool = server.send(msg_type='command', param1=command, param2=command)
        if sent:
            logging.info("Done")
        else:
            logging.error("Fail")

    # TODO: Instead of waiting for a set amount of time, (implement and) wait for an
    #       ack message from LabVIEW before closing the connection.
    #
    await asyncio.sleep(5)

if __name__ == '__main__':
    asyncio.run(main(), debug=True)
