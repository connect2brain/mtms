#!/bin/bash
set -e

if [ "$EEG_PREPROCESSOR_ENABLE" != "true" ]; then
  echo "EEG preprocessor is disabled, exiting."
  exit 1
fi

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_processor eeg_preprocessor.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  preprocessor-type:="$EEG_PREPROCESSOR_TYPE" \
  preprocessor-script:="$EEG_PREPROCESSOR_SCRIPT"
