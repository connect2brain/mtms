#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash
source /app/install/setup.bash

ros2 launch pedal_listener pedal_listener.launch.py log-level:="$ROS_LOG_LEVEL"
