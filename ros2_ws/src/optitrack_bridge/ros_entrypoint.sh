#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch optitrack_bridge optitrack_bridge.launch.py log-level:="$ROS_LOG_LEVEL" server-index:="$OPTITRACK_BRIDGE_SERVER_INDEX"
