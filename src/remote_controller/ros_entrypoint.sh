#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash
source /app/install/setup.bash

ros2 launch remote_controller remote_controller.launch.py log-level:="$ROS_LOG_LEVEL"
