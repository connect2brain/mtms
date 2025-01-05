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

For installation instructions for the computer, operating system, and external software required for the mTMS system,
see the [Installation Guide](docs/source/markdown/installation-guide.md).

After those steps, you can install the mTMS software by running the installation script:

```
source scripts/install.sh [site]
```

where valid values for `[site]` can be found by listing the contents of the `sites` directory:

```
ls sites
```

The installation script will install the mTMS software and its dependencies, including the ROS 2 workspace.

### Web UI Setup
Create a desktop link to the mTMS panel by following these steps:
   - Open Chrome and navigate to https://localhost:3001
   - Click "Install mTMS panel"
   - Enable launching for desktop shortcut

## Getting started

After the installation, you can open "mTMS panel" on the desktop to access features of the system, such as the
experiment control panel.

See the directories `api/python/examples` and `api/matlab/examples` for example scripts on how to use the APIs
to control the mTMS device.

Comprehensive API documentation can be found on a web page, to which there is a link on the computer
desktop ("mtms documentation" icon).

## Known Differences Between Installations

For site-specific configurations and known issues, see the [Known Differences](docs/source/markdown/known-differences.md) document.

## Guide for installing PREEMPT_RT kernel

For instructions on how to install the PREEMPT_RT kernel for recent kernel versions, see the [PREEMPT_RT Installation Guide](docs/source/markdown/preempt-rt-guide.md).
