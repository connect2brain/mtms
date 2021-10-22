#!/bin/sh

set -e

# Wait for the initialization to complete
./wait-for-it.sh "$INIT_HOST:$INIT_PORT" -t 60

# Run the backend
poetry run python py/backend/src/runner.py
