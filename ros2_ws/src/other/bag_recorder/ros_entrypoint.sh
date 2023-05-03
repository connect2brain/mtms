#!/bin/bash
set -e

if [ -z "$PROJECT_NAME" ]; then
    echo "Error: Project name is undefined."
    exit 1
fi

if [ -z "$BAG_NAME" ]; then
    echo "Error: Bag name is undefined."
    exit 1
fi

source /opt/ros/galactic/setup.bash
source /app/ros2_ws/install/setup.bash

ros2 bag record -a -o /app/projects/$PROJECT_NAME/bags/$(date +%Y-%m-%d_%H-%M-%S)_$BAG_NAME
