#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/python3.8/dist-packages/pypolaris


ros2 run neuronavigation_pkg start
