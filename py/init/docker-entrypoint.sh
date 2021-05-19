#!/bin/sh

set -e

./wait-for-it.sh "$KAFKA_IP:$KAFKA_PORT" -t 60

# Run the initialization
poetry run python py/init/src/runner.py

echo Initialization complete. Telling that to other services by opening port $INIT_PORT.
while true
do
  nc -lnvp $INIT_PORT
done
