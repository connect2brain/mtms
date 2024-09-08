#!/bin/bash

# Function to check if the MATLAB license server is available
check_license_server() {
  # Replace with your actual license server and port
  local server="lic-matlab.aalto.fi"
  local port="7313"

  timeout 1 bash -c "</dev/tcp/$server/$port" >/dev/null 2>&1
}

# Wait for the license server to be available if $USE_MATLAB_LICENSE_SERVER is set
if [ "$USE_MATLAB_LICENSE_SERVER" = "true" ]; then
  echo "Checking MATLAB license server..."
  until check_license_server; do
    echo "MATLAB license server is not available. Retrying in 5 seconds..."
    sleep 5
  done
  echo "MATLAB license server is available."
fi

echo "Starting ROS node in MATLAB..."
matlab -nodisplay -nosplash -nodesktop -r "run"
