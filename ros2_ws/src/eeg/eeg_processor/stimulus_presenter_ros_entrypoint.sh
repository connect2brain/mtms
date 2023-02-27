#!/bin/bash
set -e

/root/miniconda3/bin/conda init bash
/root/miniconda3/bin/conda activate default

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_processor stimulus_presenter.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  stimulus-presenter-type:="$STIMULUS_PRESENTER_TYPE" \
  stimulus-presenter-script:="$STIMULUS_PRESENTER_SCRIPT"
