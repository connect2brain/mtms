#!/bin/bash

echo "Pushing okahilak/mtms:fpga_bridge"
docker push okahilak/mtms:fpga_bridge

echo "Pushing okahilak/mtms:eeg_bridge"
docker push okahilak/mtms:eeg_bridge

echo "Pushing okahilak/mtms:eeg_processor"
docker push okahilak/mtms:eeg_processor

echo "Pushing okahilak/mtms:rosbridge"
docker push okahilak/mtms:rosbridge

echo "Pushing okahilak/mtms:planner"
docker push okahilak/mtms:planner

echo "Pushing okahilak/mtms:pulse_sequence_controller"
docker push okahilak/mtms:pulse_sequence_controller

echo "Pushing okahilak/mtms:eeg_simulator"
docker push okahilak/mtms:eeg_simulator

echo "Pushing okahilak/mtms:data_batcher"
docker push okahilak/mtms:data_batcher

echo "Pushing okahilak/mtms:front"
docker push okahilak/mtms:front

echo "Pushing okahilak/mtms:optitrack_bridge"
docker push okahilak/mtms:optitrack_bridge
