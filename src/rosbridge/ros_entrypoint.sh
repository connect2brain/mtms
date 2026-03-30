#!/bin/bash
set -e

source /opt/ros/jazzy/setup.bash
source /app/install/setup.bash

TOPICS_GLOB="\"[\
'/mtms/trial/trial_readiness',\
'/mtms/device/system_state',\
'/mtms/device/session',\
'/mtms/experiment/state',\
'/mtms/experiment/feedback',\
'/mtms/remote_controller/state',\
'/mtms/eeg_device/info',\
'/mtms/eeg/healthcheck',\
'/mtms/device/healthcheck',\
'/mtms/mep/healthcheck',\
'/mtms/busylight_manager/heartbeat',\
'/mtms/experiment_performer/heartbeat',\
'/mtms/pedal_listener/heartbeat',\
'/mtms/trial_logger/heartbeat',\
'/mtms/voltage_setter/heartbeat',\
'/mtms/waveform_approximator/heartbeat',\
'/mtms/waveform_utils/get_default_waveform/heartbeat',\
'/mtms/waveform_utils/get_multipulse_waveforms/heartbeat',\
'/mtms/device/heartbeat',\
'/mtms/eeg_bridge/heartbeat',\
'/mtms/trigger_processor/heartbeat',\
'/mtms/remote_controller/heartbeat',\
'/mtms/targeting/heartbeat',\
'/mtms/stimulation_allower/heartbeat',\
'/mtms/timebase_calibrator/heartbeat',\
'/mtms/trial_performer/heartbeat'\
]\""

SERVICES_GLOB="\"[\
'/mtms/remote_controller/start',\
'/mtms/remote_controller/stop',\
'/mtms/device/start',\
'/mtms/device/stop',\
'/mtms/targeting/get_maximum_intensity',\
'/mtms/experiment/count_valid_trials',\
'/mtms/experiment/perform',\
'/neuronavigation/visualize/targets',\
'/mtms/experiment/pause',\
'/mtms/experiment/resume',\
'/mtms/experiment/cancel'\
]\""

ros2 launch rosbridge_server rosbridge_websocket_launch.xml \
  port:=$ROSBRIDGE_PORT \
  namespace:=mtms \
  topics_glob:="$TOPICS_GLOB" \
  services_glob:="$SERVICES_GLOB" \
  params_glob:="[]"
