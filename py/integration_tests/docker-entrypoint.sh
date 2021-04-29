#!/bin/sh

set -e

./wait-for-it.sh "$BACKEND_HOST:$BACKEND_PORT" -t 60

pipenv run pytest
