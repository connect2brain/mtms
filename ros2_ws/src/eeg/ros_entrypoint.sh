#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg eeg_bridge_launch.py
