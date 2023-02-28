# Installation
This document describes the steps required for installing the environment for the host machine that is connected to the mTMS device.

1. Install Ubuntu 22.04.
2. Install real-time kernel patch via Ubuntu Pro live patching.
3. Install docker, `mtms/scripts/install_docker.sh`.
4. Install ROS2, `mtms/scripts/install_ros.sh`.
5. Install FPGA drivers. See instructions from https://www.ni.com/fi-fi/support/documentation/supplemental/18/downloading-and-installing-ni-driver-software-on-linux-desktop.html. On step 4, install packages `ni-rseries` and `ni-fpga-interface`.
6. Configure network to receive data from the EEG device. Open settings -> network -> wired options -> IPv4 -> IPv4 Method, select Manual. Set address to 192.168.200.221 and Netmask to 255.255.255.0. Apply.
7. Modify `/etc/security/limits.conf` and add the following lines: 
    ```
    <your username>    -   rtprio    98
    <your username>    -   memlock   1000000
    ```
