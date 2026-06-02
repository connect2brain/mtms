#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash
source /app/install/setup.bash

PYPOLARIS_PATH=$(python3 -c "import pypolaris, os; print(os.path.dirname(pypolaris.__file__))" 2>/dev/null || true)

if [ -n "$PYPOLARIS_PATH" ]; then
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PYPOLARIS_PATH"
fi

ros2 run neuronavigation start --ros-args -p electric_field_enable:="${ELECTRIC_FIELD_ENABLE:-false}"
