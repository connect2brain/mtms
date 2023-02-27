#!/bin/bash
set -e

/root/miniconda3/bin/conda init bash
/root/miniconda3/bin/conda activate default

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_processor eeg_preprocessor.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  preprocessor-type:="$EEG_PREPROCESSOR_TYPE" \
  preprocessor-script:="$EEG_PREPROCESSOR_SCRIPT"
