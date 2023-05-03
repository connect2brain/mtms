#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

if [ -z "$BAG_NAME" ]; then
  echo "Error: BAG_NAME is unset in .env file."
  exit 1
fi

if [ -z "$BAG_TOPIC" ]; then
  echo "Error: BAG_TOPIC is unset in .env file."
  exit 1
fi

if [ -z "$BAG_TIMESTAMP" ]; then
  python3 bag_exporter.py --bag $BAG_NAME --topic $BAG_TOPIC --full
else
  python3 bag_exporter.py --bag $BAG_NAME --timestamp $BAG_TIMESTAMP --topic $BAG_TOPIC --full
fi
