#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Kafka commands shared across modules.

KAFKA_COMMAND_SET_STIMULATION_PARAMETERS: str = 'stimulation.set_parameters'
KAFKA_COMMAND_SET_COIL_AT_TARGET: str = 'stimulation.set_coil_at_target'

# Other constants.

STIMULATION_PARAMETERS = [
    'intensity',
    'isi',
]
