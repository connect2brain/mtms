#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

cleanup () {
  docker-compose -p test kill
  docker-compose -p test rm -f
}
trap 'cleanup' HUP INT QUIT PIPE TERM

printf "Starting all services\n"
docker-compose --env-file .env.test -p test -f docker-compose.yml up -d

# Run within-service integration tests that depend on the other services running.
printf "Running tests for EEG service\n"
docker-compose --env-file .env.test -p test -f docker-compose.yml exec eeg ./integration-tests.sh
EXIT_CODE_EEG=$?

# Run tester service that simulates the user interaction with the system.
printf "Running tests for user interaction\n"
docker-compose --env-file .env.test -p test -f docker-compose.yml -f docker-compose.tester.yml up --exit-code-from tester tester
EXIT_CODE_TESTER=$?

printf "Shutting down all services\n"
docker-compose --env-file .env.test -p test -f docker-compose.yml -f docker-compose.tester.yml down

if [ $EXIT_CODE_EEG -eq 0 ] && [ $EXIT_CODE_TESTER -eq 0 ] ; then
  printf "${GREEN}Tests passed${NC}\n"
else
  printf "${RED}Tests failed${NC}\n"
fi

exit $EXIT_CODE
