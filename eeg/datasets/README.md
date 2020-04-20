# Datasets

This folder contains links to datasets that can be used for eeg simulation.

## Dataset download

A script called `dataset_download.sh` is provided. It pertially relies on tools called `openneuro-cli`,  `datalad` and `git-annex` to download datasets. 

### Install openneuro-cli

Datasets can be downloaded manually from Openneuro.org, or using their Command Line Interface tool `openneuro-cli`.

Prerequisite: NodeJS (version 10 or higher) and `npm`

To install `openneuro-cli` globally:

	npm install -g openneuro-cli

First, login to openneuro.org using your browser and generate an API key: [https://openneuro.org/keygen] (only works when you are logged in).

Then, login using command line:

	openneuro login

Select openneuro.org and paste in the API key you created.

Usage example:

	openneuro download --snapshot 1.3.0 ds000113 forrestgump/

### Install git-annex

Git annex can be used to download some datasets from, for example, github. 

MacOS: 

	brew install git-annex

Linux (Debian/Ubuntu):

	sudo apt-get install git-annex

Others: See [git-annex.branchable.com/install/](https://git-annex.branchable.com/install/).

### Install Datalad

Datalad is another automated tool that can download datasets.

MacOS:

	pip install datalad

Linux (Debian/Ubuntu):

	pip install datalad

or

	sudo apt-get install datalad


