#!/bin/bash

# Validate license configuration first.
if [ "$USE_MATLAB_LICENSE_SERVER" = "true" ]; then
  if [ -z "$MATLAB_LICENSE_SERVER" ]; then
    echo "USE_MATLAB_LICENSE_SERVER=true, but MATLAB_LICENSE_SERVER is empty."
    exit 1
  fi
elif [ "$USE_MATLAB_LICENSE_SERVER" = "false" ]; then
  if [ -z "$MATLAB_LICENSE_FILE" ]; then
    echo "USE_MATLAB_LICENSE_SERVER=false, but MATLAB_LICENSE_FILE is empty."
    exit 1
  fi
else
  echo "USE_MATLAB_LICENSE_SERVER must be either 'true' or 'false'."
  exit 1
fi

# Function to check if the MATLAB license server is available
check_license_server() {
  local license_server="$MATLAB_LICENSE_SERVER"
  local port="${license_server%%@*}"
  local server="${license_server#*@}"

  if [ -z "$port" ] || [ -z "$server" ] || [ "$license_server" = "$server" ]; then
    echo "Invalid MATLAB license server format: '$license_server' (expected port@host)."
    return 1
  fi

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

# Dispatch explicit env vars to MLM_LICENSE_FILE variable, used by MATLAB.
if [ "$USE_MATLAB_LICENSE_SERVER" = "true" ]; then
  export MLM_LICENSE_FILE="$MATLAB_LICENSE_SERVER"
  echo "Using MATLAB network license server: $MLM_LICENSE_FILE"
else
  export MLM_LICENSE_FILE="$MATLAB_LICENSE_FILE"
  echo "Using MATLAB local license file: $MLM_LICENSE_FILE"
fi

echo "Starting ROS node in MATLAB..."

# XXX: Starting a full MATLAB session with the command
#
#  matlab -nodisplay -nosplash -nodesktop -r "run"
#
# seems to be very slow, taking more than 1 minute to start in MATLAB
# version R2024b. Instead, we can use the -batch option to run a MATLAB
# script directly, which seems to be much faster.
#
# TODO: Has this been fixed in MATLAB R2025b? If so, ROS messages could
#   be registered in ros_entrypoint.sh instead of run.m.
matlab -batch "run"
