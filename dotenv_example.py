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

# Buffer length in samples
BACKEND_BUFFER_LENGTH = 1024

EDF_TEST_FILE='datasets/MNE-eegbci-data/files/eegmmidb/1.0.0/S001/S001R01.edf'
KAFKA_IP='127.0.0.1'
KAFKA_PORT='9092'
KAFKA_TOPIC='eeg_data'
