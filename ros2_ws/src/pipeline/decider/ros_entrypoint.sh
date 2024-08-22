#!/bin/bash

# Run CUDA test script if CUDA is enabled.
if [ -n "$USE_CUDA" ]; then
    echo "Testing CUDA..."
    python3 /app/ros2_ws/src/pipeline/decider/test_cuda.py

    success=$?
    if [ $success -ne 0 ]; then
        echo "CUDA test failed, please disable USE_CUDA environment variable"
        exit $success
    fi
fi

set -e

source /opt/ros/iron/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 launch decider decider.launch.py log-level:="$ROS_LOG_LEVEL" minimum-intertrial-interval:="$MINIMUM_INTERTRIAL_INTERVAL"
