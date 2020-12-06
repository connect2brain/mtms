# Prefix a comment with `##' to show the comment when running `make help'.

## Use `make [make-target]' to start different parts of the system.
##
## A list of make targets:

help:                   ## Show this help.
	@sed -ne '/@sed/!s/## //p' $(MAKEFILE_LIST)
.PHONY: help

init-env:               ## Initialize environment variables.
	cp dotenv_example.py .env
.PHONY: init-env

init-db:                ## Initialize database.
	cd db && \
	rm -f topics.db && \
	sqlite3 topics.db ".read topics.sql" && \
	chmod a-w topics.db
.PHONY: init-db

init: init-env init-db  ## Initialize all.
.PHONY: init

install-frontend:       ## Install frontend.
	cd frontend && npm install
.PHONY: install-frontend

run-zookeeper-win:      ## Run Zookeeper in Windows.
	scripts\run-zookeeper.bat
.PHONY: run-zookeeper-win

run-kafka-win:          ## Run Kafka in Windows.
	scripts\run-kafka.bat
.PHONY: run-kafka-win

run-backend:            ## Run backend.
	pipenv run python -m backend.runner
.PHONY: run-backend

run-frontend:           ## Run frontend.
	cd frontend && npm run serve
.PHONY: run-frontend

run-kafka-bridge:       ## Run Kafka-LabVIEW bridge.
	pipenv run python -m bridge.runner
.PHONY: run-kafka-bridge

download-data:          ## Download EEG data.
                        ## Use DATASET environment variable to specify the dataset.
                        ## See eeg/datasets/README.md for more detailed instructions.
                        ## Example: make download-data DATASET=eegbci
	cd eeg/datasets && sh dataset_download.sh $(DATASET)
.PHONY: download-data

stream-data:            ## Stream EEG data via Kafka.
                        ## Use DATASET_FILE environment variable to specify the dataset file.
                        ## Example: make stream-data DATASET_FILE=eeg/datasets/MNE-eegbci-data/files/eegmmidb/1.0.0/S001/S001R01.edf
	pipenv run python -m eeg.send_eeg_over_kafka $(DATASET_FILE)
.PHONY: stream-data

reset-env:              ## Reset .env file to the default.
	cp dotenv_example.py .env
.PHONY: reset-env
