# Datasets

This folder contains links to datasets that can be used for eeg simulation.

## Dataset download

A script called `dataset_download.sh` is provided. It partially relies on tools called `openneuro-cli`,  `datalad` and `git-annex` to download datasets.

Note that if setting up openneuro-cli, using the instructions below, is not complete, the script crashes when attempting to use openneuro-cli for downloading.

### Install openneuro-cli

Datasets can be downloaded manually from Openneuro.org, or using their Command Line Interface tool `openneuro-cli`.

Prerequisite: NodeJS (version 10 or higher) and `npm`

To install `openneuro-cli` globally:

	npm install -g openneuro-cli

Then, set up openneuro-cli by doing the following steps:

1. Login to openneuro.org using your browser

2. Generate an API key: [https://openneuro.org/keygen] (only works when you are logged in).

3. Login using command line:

	openneuro login

4. Select openneuro.org and paste in the API key you created.

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
