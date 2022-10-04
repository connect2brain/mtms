#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch data_provider data_provider.launch.py log-level:="$ROS_LOG_LEVEL" data-file:="$DATA_PROVIDER_DATA_FILE_NAME"
