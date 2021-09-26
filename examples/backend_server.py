#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example
# -------
#
# Initialize by:
#
#     poetry install
#
# Run by:
#
#     poetry run python backend_server.py
#
# Creates a Socket.IO server and sends messages to InVesalius based on
# keyboard input.
#

import aioconsole
import asyncio
import json
import nest_asyncio
import socketio
import time
import uvicorn

nest_asyncio.apply()

backend_port = 5000
connected = False

sio = socketio.AsyncServer(async_mode='asgi')
app = socketio.ASGIApp(sio)

@sio.event
def connect(sid, environ):
    print('connected to ', sid)

    global connected
    connected = True

def print_json_error(e):
    print("Invalid JSON")
    print(e.doc)
    print(" " * e.pos + "^")
    print(e.msg)
    print("")

async def run():
    while True:
        if not connected:
            await asyncio.sleep(1)
            continue

        print("Enter topic: ")
        topic = await aioconsole.ainput()
        print("Enter data as JSON: ")
        data = await aioconsole.ainput()

        try:
            decoded = json.loads(data)
        except json.decoder.JSONDecodeError as e:
            print_json_error(e)
            continue

        await sio.emit(
            event="to_neuronavigation",
            data={
                "topic": topic,
                "data": decoded,
            }
        )

async def main():
    asyncio.create_task(run())
    uvicorn.run(app, port=backend_port, host='0.0.0.0', loop='asyncio')

if __name__ == '__main__':
    asyncio.run(main(), debug=True)
