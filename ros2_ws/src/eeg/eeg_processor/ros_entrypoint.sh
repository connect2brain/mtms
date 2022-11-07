#!/bin/bash
set -e

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

PROCESSOR_SCRIPT=hotswappable_processors.python.python_processor

if [ "$EEG_PROCESSOR_TYPE" = "compiledmatlab" ]; then
  PROCESSOR_SCRIPT=hotswappable_processors/cppmatlab/compiler/libprocessor_factory.so
fi


ros2 launch eeg_processor eeg_processor.launch.py \
    log-level:="$ROS_LOG_LEVEL" \
    processor-type:="$EEG_PROCESSOR_TYPE" \
    processor-script:="$PROCESSOR_SCRIPT" \
    loop-count:="$EEG_PROCESSOR_TEST_MEASURE_COUNT" \
    file:="$EEG_PROCESSOR_DURATION_FILE"
