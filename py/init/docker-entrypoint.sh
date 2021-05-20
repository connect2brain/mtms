#!/bin/sh

set -e

./wait-for-it.sh "$KAFKA_IP:$KAFKA_PORT" -t 60

poetry run python py/init/src/runner.py
