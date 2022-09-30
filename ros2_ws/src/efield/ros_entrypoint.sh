#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/app/ros2_ws/src/efield/efield_libraries/lib/

ros2 run efield efield_service
