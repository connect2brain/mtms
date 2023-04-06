#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_simulator eeg_simulator.launch.py log-level:="$ROS_LOG_LEVEL" data-file:="$EEG_SIMULATOR_DATA_FILE_NAME" sampling-frequency:="$EEG_SIMULATOR_SAMPLING_FREQUENCY" loop:="true" eeg-channels:="$EEG_SIMULATOR_NUMBER_OF_EEG_CHANNELS"
