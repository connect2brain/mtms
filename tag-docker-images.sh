#!/bin/bash

docker tag mtms_mtms_device_bridge okahilak/mtms:mtms_device_bridge
docker tag mtms_eeg_bridge okahilak/mtms:eeg_bridge
docker tag mtms_eeg_processor okahilak/mtms:eeg_processor
docker tag mtms_rosbridge okahilak/mtms:rosbridge
docker tag mtms_planner okahilak/mtms:planner
docker tag mtms_pulse_sequence_controller okahilak/mtms:pulse_sequence_controller
docker tag mtms_eeg_simulator okahilak/mtms:eeg_simulator
docker tag mtms_eeg_batcher okahilak/mtms:eeg_batcher
docker tag mtms_front okahilak/mtms:front
docker tag mtms_optitrack_bridge okahilak/mtms:optitrack_bridge
