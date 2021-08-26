#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Kafka commands shared across modules.

KAFKA_COMMAND_SET_COIL_AT_TARGET: str = 'stimulation.set_coil_at_target'
KAFKA_COMMAND_SET_STIMULATION_PARAMETERS: str = 'stimulation.set_parameters'

KAFKA_COMMAND_SET_PEDAL_CONNECTION: str = 'status.set_pedal_connection'
KAFKA_COMMAND_SET_SERIAL_PORT_CONNECTION: str = 'status.set_serial_port_connection'

# Other constants.

STIMULATION_PARAMETERS = [
    'intensity',
    'iti',
]
