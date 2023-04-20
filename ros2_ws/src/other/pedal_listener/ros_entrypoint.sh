#!/bin/bash
set -e

source /opt/ros/humble/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 run pedal_listener start
