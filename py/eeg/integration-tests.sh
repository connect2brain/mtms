#!/bin/sh

set -e

./wait-for-it.sh "$BACKEND_HOST:$BACKEND_PORT" -t 60

# Test streaming an EEG dataset
pipenv run python py/eeg/src/runner.py datasets/eeg_test.edf
