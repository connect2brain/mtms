# EEG preprocessing routines

## Example scripts:

basic_preprocessing_example_script.m is an example script for (offline) cleaning standard TMS-EEG data recorded with the Brainvision system. 

## Data: 

The script assumes the data in the EEGLAB data format. An example dataset
is provided in the C2B project folder. 

//tw-nbe.org.aalto.fi/project/c2b-shared/SW_development/TMS_EEG_testset.set

## Dependencies: 

1. Matlab, tested with R2018b

1. Fastica package, version  https://research.ics.aalto.fi/ica/fastica/

2. EEGLAB toolbox, tested with version 2019_0 https://sccn.ucsd.edu/eeglab/index.php

3. TESA EEGLAB plugin, version 1.0.1 https://nigelrogasch.github.io/TESA/

## Installation:

1. Download and extract the Fastica package. Define the path to the Fastica package in the beginning of the Matlab script.

2. Download and extract EEGLAB package. Define the path to the EEGLAB package in the beginning of the Matlab script.

3. Download and extract the TESA package to ~/path_to_eeglab/eeglab2019_0/plugins

Tuomas Mutanen 7.5.2020