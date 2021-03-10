#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

cleanup () {
  docker-compose -p test kill
  docker-compose -p test rm -f
}
trap 'cleanup' HUP INT QUIT PIPE TERM

docker-compose --env-file .env.test -p test -f docker-compose.yml -f docker-compose.test.yml build tester

docker-compose --env-file .env.test -p test -f docker-compose.yml -f docker-compose.test.yml up --exit-code-from tester tester
EXIT_CODE=$?

docker-compose --env-file .env.test -p test -f docker-compose.yml -f docker-compose.test.yml down

if [ $EXIT_CODE -eq 0 ] ; then
  printf "${GREEN}Tests passed${NC}\n"
else
  printf "${RED}Tests failed${NC}\n"
fi

exit $EXIT_CODE
