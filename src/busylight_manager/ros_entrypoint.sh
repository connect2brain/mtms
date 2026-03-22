#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash
source /app/install/setup.bash

ros2 launch busylight_manager busylight_manager.launch.py log-level:="$ROS_LOG_LEVEL"
