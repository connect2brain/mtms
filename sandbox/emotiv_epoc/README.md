# Publishing EEG data from Emotiv Epoc+

This example script reads EEG data from Emotiv Epoc+ and publishes it via Kafka
in a topic named 'eeg_raw'.

Emotiv Epoc+ is a consumer-grade EEG device. One such device is owned by the
author of the script, Olli-Pekka Kahilakoski.

## Prerequisites

Python >= 3.6 is required by Cortex API, provided by Emotiv, used to access
the device.

Cortex API is only available for Windows and MacOS environments.

Cortex API version 2.0 is required.

## Example use

It is good practice to create the Kafka topic in advance, as pykafka does
not have an API for creating new topics. However, if pykafka is used to
access a topic that does not exist, it will be created implicitly.

Here is how to run the script from Windows command line:

    kafka-topics.bat --create --bootstrap-server 127.0.0.1:8082 --topic eeg_raw --replication-factor 1 --partitions 1
    poetry install
    poetry run python publish_eeg.py
