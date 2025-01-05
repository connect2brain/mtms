# mTMS project

## Copyright and permitted use

All rights to the code in this repository (excluding the submodules) belongs to Aalto University.

Currently, its permitted use is limited to people in Connect2Brain project in Aalto University, the Eberhard Karl University of Tübingen,
and D'Annunzio University of Chieti–Pescara, and Aalto University students and employees performing research assignments in Aalto C2B laboratory
or developing the software under a non-disclosure agreement.

The software also uses external software and libraries, which reside in their own repositories, are connected to this
repository using Git submodules, and have their own copyright owners. Here is the list of the external repositories
and where they are located in the directory structure:

[InVesalius3](https://github.com/invesalius/invesalius3), `ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3`

e-field library, `ros2_ws/src/mtms_packages/targeting/efield/src`

Please see the repository roots of the external repositories for their respective authors and licenses.

## Installation Guide

For detailed installation instructions, see the [Installation Guide](docs/source/markdown/installation-guide.md).

## Getting started

After the installation, you can open "mTMS panel" on the desktop to access some features of the system, such as the real-time pipeline.

Other features, such as the Python and MATLAB APIs to control the mTMS device, are documented on
a web page, to which there is a link on the computer desktop ("mtms documentation" icon).

## Known Differences Between Installations

For site-specific configurations and known issues, see the [Known Differences](docs/source/markdown/known-differences.md) document.

## Guide for installing PREEMPT_RT kernel

For instructions on how to install the PREEMPT_RT kernel for recent kernel versions, see the [PREEMPT_RT Installation Guide](docs/source/markdown/preempt-rt-guide.md).
