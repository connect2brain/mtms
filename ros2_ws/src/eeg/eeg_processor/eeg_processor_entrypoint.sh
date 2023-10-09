#!/bin/bash
set -e

if [ "$EEG_PROCESSOR_ENABLE" != "true" ]; then
  echo "EEG processor is disabled, exiting."
  exit 1
fi

source /opt/ros/iron/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_processor eeg_processor.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  processor-type:="$EEG_PROCESSOR_TYPE" \
  processor-script:="$EEG_PROCESSOR_SCRIPT" \
  preprocess:="$EEG_PREPROCESSOR_ENABLE"
