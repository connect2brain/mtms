#!/bin/bash

echo "Registering ROS messages in MATLAB..."
matlab -batch "ros2RegisterMessages('/home/matlab');"

success=$?
if [ $success -ne 0 ]; then
    echo ""
    echo "Failed to register ROS messages in MATLAB. Please double-check the value of \$MLM_LICENSE_FILE in .env."
    exit 1
fi

echo "Starting ROS node in MATLAB..."
matlab -nodisplay -nosplash -nodesktop -r "run"
