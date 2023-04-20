#!/bin/bash
set -e

source /opt/ros/humble/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_processor stimulus_presenter.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  stimulus-presenter-type:="$STIMULUS_PRESENTER_TYPE" \
  stimulus-presenter-script:="$STIMULUS_PRESENTER_SCRIPT"
