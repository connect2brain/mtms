#!/bin/bash
set -e

if [ "$STIMULUS_PRESENTER_ENABLE" != "true" ]; then
  echo "Stimulus presenter is disabled, exiting."
  exit 1
fi

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_processor stimulus_presenter.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  stimulus-presenter-type:="$STIMULUS_PRESENTER_TYPE" \
  stimulus-presenter-script:="$STIMULUS_PRESENTER_SCRIPT"
