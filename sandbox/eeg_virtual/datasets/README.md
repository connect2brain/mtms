# Datasets

This folder contains links to datasets that can be used for eeg simulation.

## Dataset download

A script called `dataset_download.sh` is provided. It relies on a tools called `datalad` and `git-annex` to download datasets.

### Install git-annex

MacOS: 

	brew install git-annex

Linux (Debian/Ubuntu):

	sudo apt-get install git-annex

Others: See [git-annex.branchable.com/install/](https://git-annex.branchable.com/install/).

### Install Datalad

MacOS:

	pip install datalad

Linux (Debian/Ubuntu):

	pip install datalad

or

	sudo apt-get install datalad

## Manual download

Datasets can be downloaded manually from Openneuro.org, or using their Command Line Interface tool `openneuro-cli`.

Prerequisite: NodeJS (version 10 or higher) and `npm`

To install `openneuro-cli` globally:

	npm install -g openneuro-cli

Usage example:

	openneuro download --snapshot 1.3.0 ds000113 forrestgump/

## Dataset descriptions

### "Forrest Gump"

DOI: [10.18112/openneuro.ds000113.v1.3.0](https://dx.doi.org/10.18112/openneuro.ds000113.v1.3.0)

https://openneuro.org/datasets/ds000113/versions/1.3.0

License: Not stated

This dataset contains high-resolution functional magnetic resonance (fMRI) data from 
20 participants recorded at high field strength (7 Tesla) during prolonged stimulation 
with an auditory feature film ("Forrest Gump''). In addition, a comprehensive set of 
auxiliary data (T1w, T2w, DTI, susceptibility-weighted image, angiography) as well as 
measurements to assess technical and physiological noise components have been acquired. 

### Resting State - TMS

DOI: [10.18112/openneuro.ds001832.v1.0.1](https://dx.doi.org/10.18112/openneuro.ds001832.v1.0.1)

https://openneuro.org/datasets/ds001832/versions/1.0.1

License: CC0

Alkhasli et al., 2019: Modulation of fronto-striatal functional connectivity using transcranial magnetic stimulation.


