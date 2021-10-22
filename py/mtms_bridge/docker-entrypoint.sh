#!/bin/sh

set -e

# Wait for the initialization to complete
./wait-for-it.sh "$INIT_HOST:$INIT_PORT" -t 60

# Run the mTMS bridge
poetry run python py/mtms_bridge/src/python_runner.py
