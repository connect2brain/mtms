# Prefix a comment with `##' to show the comment when running `make help'.

## Use `make [make-target]' to start different parts of the system.
##
## A list of make targets:

help:                 ## Show this help.
	@sed -ne '/@sed/!s/## //p' $(MAKEFILE_LIST)

run-zookeeper-win:    ## Run Zookeeper in Windows.
	scripts\run-zookeeper.bat

run-kafka-win:        ## Run Kafka in Windows.
	scripts\run-kafka.bat

run-backend:          ## Run backend.
	pipenv run python -m backend.runner

run-frontend:         ## Run frontend.
	cd frontend && npm run serve

install-frontend:     ## Install frontend.
	cd frontend && npm install

download-data:        ## Download EEG data.
                      ## Use DATASET environment variable to specify the dataset.
                      ## See eeg/datasets/README.md for more detailed instructions.
                      ## Example: make download-data DATASET=eegbci
	cd eeg/datasets && sh dataset_download.sh $(DATASET)

stream-data:          ## Stream EEG data via Kafka.
                      ## Use DATASET_FILE environment variable to specify the dataset file.
                      ## Example: make stream-data DATASET_FILE=eeg/datasets/MNE-eegbci-data/files/eegmmidb/1.0.0/S001/S001R01.edf
	pipenv run python -m eeg.send_eeg_over_kafka $(DATASET_FILE)

reset-env:            ## Reset .env file to the default.
	cp dotenv_example.py .env
