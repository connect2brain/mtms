#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch data_batcher data_batcher.launch.py log-level:="$ROS_LOG_LEVEL" batch-size:="$DATA_BATCHER_BATCH_SIZE" downsample-ratio:="$DATA_BATCHER_DOWNSAMPLE_RATIO"
