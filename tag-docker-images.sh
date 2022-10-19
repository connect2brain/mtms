#!/bin/bash

docker tag mtms_fpga_bridge okahilak/mtms:fpga_bridge
docker tag mtms_eeg_bridge okahilak/mtms:eeg_bridge
docker tag mtms_eeg_processor okahilak/mtms:eeg_processor
docker tag mtms_rosbridge okahilak/mtms:rosbridge
docker tag mtms_planner okahilak/mtms:planner
docker tag mtms_pulse_sequence_controller okahilak/mtms:pulse_sequence_controller
docker tag mtms_eeg_simulator okahilak/mtms:eeg_simulator
docker tag mtms_data_batcher okahilak/mtms:data_batcher
docker tag mtms_front okahilak/mtms:front
