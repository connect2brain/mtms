#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 run eeg_processor eeg_processor
