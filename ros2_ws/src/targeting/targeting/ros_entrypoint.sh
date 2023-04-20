#!/bin/bash
set -e

source /opt/ros/humble/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch targeting targeting.launch.py log-level:="$ROS_LOG_LEVEL" site:="$SITE"
