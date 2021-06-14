#!/bin/sh

set -e

./wait-for-it.sh "$KAFKA_IP:$KAFKA_PORT" -t 60

# Run the initialization
poetry run python py/init/src/runner.py

# XXX: Consider if there should be a more explicit way of communicating that the initialization is finished.
#      The benefit of just opening the port is that it is simple, but it could arguably be more explicit.
#
echo Initialization complete. Telling that to other services by opening port $INIT_PORT.

# The infinite loop is here because a single execution of netcat terminates after the connection to the other
# end is closed.
while true
do
  nc -lnvp $INIT_PORT
done
