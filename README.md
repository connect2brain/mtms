# mTMS project

## Copyright and permitted use

All rights to the code in this repository (excluding the submodules) belongs to Aalto University.

Currently, its permitted use is limited to people in Connect2Brain project in Aalto University, the Eberhard Karl University of Tübingen,
and D'Annunzio University of Chieti–Pescara, and Aalto University students and employees performing research assignments in Aalto C2B laboratory
or developing the software under a non-disclosure agreement.

The software also uses external software and libraries, which reside in their own repositories, are connected to this
repository using Git submodules, and have their own copyright owners. Here is the list of the external repositories
and where they are located in the directory structure:

[InVesalius3](https://github.com/invesalius/invesalius3), `ros2_ws/src/neuronavigation/neuronavigation/invesalius3`

[rosbridge_suite](https://github.com/RobotWebTools/rosbridge_suite/), `ros2_ws/src/bridges/rosbridge_suite`

e-field library, `ros2_ws/src/targeting/efield/src`

Please see the repository roots of the external repositories for their respective authors and licenses.

## Installation

The mTMS software is intended to be used with a dedicated computer with real-time Ubuntu installed.

Steps for installing and configuring the system and the software can be found under
`Groups/SW Group/Computer installation` folder in ConnectToBrain Google Drive.

After those steps, re-booting the computer automatically starts the Docker containers that run the mTMS software.

## Getting started

After the installation, you can open "mTMS panel" on the desktop to access some features of the system, such as the real-time pipeline.

Other features, such as the Python and MATLAB APIs to control the mTMS device, are documented in "mtms documentation" file, which can be found on the computer desktop after following the installation instructions.
