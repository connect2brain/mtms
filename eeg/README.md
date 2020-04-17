# Stream EEG data

## Installation

Make sure that you have Python 3.6 installed on your system. 

### Install Python 3.6 using Pyenv

	pyenv install -v 3.6.10
	pyenv local 3.6.10

### Install required packages

	pipenv install

## Download data

	cd datasets
	sh dataset_download.sh eegbci
	cd ..

## Send data over Kafka (example)

	pipenv run python send_eeg_over_kafka.py datasets/MNE-eegbci-data/files/eegmmidb/1.0.0/S001/S001R01.edf S001R02.edf S001R03.edf

