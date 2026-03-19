#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash
source /app/install/setup.bash

ros2 launch timebase_calibrator timebase_calibrator.launch.py log-level:="$ROS_LOG_LEVEL"
