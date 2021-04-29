# Prefix a comment with `##' to show the comment when running `make help'.

## Use `make [make-target]' to start different parts of the system.
##
## A list of make targets:

help:                   ## Show this help.
	@sed -ne '/@sed/!s/## //p' $(MAKEFILE_LIST)
.PHONY: help

reset-env:              ## Reset environment variables.
	cp .env.default .env
.PHONY: reset-env

test-backend:           ## Run backend tests.
	docker-compose -p test build backend && docker-compose -p test run backend pipenv run pytest
.PHONY: test-backend

integration-tests:      ## Run integration tests.
	docker-compose -f docker-compose.yml -f docker-compose.test.yml run tester
.PHONY: integration-tests

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
