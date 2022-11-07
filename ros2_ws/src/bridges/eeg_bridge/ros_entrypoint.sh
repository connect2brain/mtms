#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_bridge eeg_bridge.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  port:="$EEG_PORT" \
  eeg-channels-primary-amplifier:="$EEG_CHANNELS_PRIMARY_AMPLIFIER" \
  emg-channels-primary-amplifier:="$EMG_CHANNELS_PRIMARY_AMPLIFIER" \
  eeg-channels-secondary-amplifier:="$EEG_CHANNELS_SECONDARY_AMPLIFIER" \
  emg-channels-secondary-amplifier:="$EMG_CHANNELS_SECONDARY_AMPLIFIER"
