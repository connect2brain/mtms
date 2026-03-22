#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash
source /app/install/setup.bash

if [ "$USE_MTMS_SIMULATOR" != "true" ]; then
  echo "USE_MTMS_SIMULATOR is not set to true, exiting."
  exit 0
fi

ros2 launch mtms_simulator mtms_simulator.launch.py log-level:="$ROS_LOG_LEVEL"
