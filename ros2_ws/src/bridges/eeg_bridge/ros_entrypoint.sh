#!/bin/bash
set -e

source /opt/ros/iron/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch eeg_bridge eeg_bridge.launch.py \
  log-level:="$ROS_LOG_LEVEL" \
  port:="$EEG_PORT" \
  number-of-eeg-channels-amplifier-1:="$NUMBER_OF_EEG_CHANNELS_AMPLIFIER_1" \
  number-of-emg-channels-amplifier-1:="$NUMBER_OF_EMG_CHANNELS_AMPLIFIER_1" \
  number-of-eeg-channels-amplifier-2:="$NUMBER_OF_EEG_CHANNELS_AMPLIFIER_2" \
  number-of-emg-channels-amplifier-2:="$NUMBER_OF_EMG_CHANNELS_AMPLIFIER_2"
