#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *****************************************************
# 
# NOTE! This is an example .env file
# 
# Copy this file to '.env' and modify contents 
# according to the current setup.
# 
# See: https://pypi.org/project/python-dotenv/
# 
# *****************************************************

# EEG settings
BACKEND_EEG_BUFFER_LENGTH = 1024

# Kafka settings
KAFKA_IP='127.0.0.1'
KAFKA_PORT='9092'

# MTMS application
MTMS_APP_NAME='MTMSActiveXServer'
MTMS_APP_LOCATION='C:\Users\okahilak\Builds\mTMS 3.0\mTMS ActiveX Server'
MTMS_APP_FILENAME='mTMS ActiveX Server.exe'
MTMS_VI_NAME='mTMS ActiveX Server.vi'

# Miscellaneous
EDF_TEST_FILE='datasets/MNE-eegbci-data/files/eegmmidb/1.0.0/S001/S001R01.edf'