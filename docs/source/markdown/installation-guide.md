# mTMS Computer Installation Guide

This guide provides step-by-step instructions for installing and configuring the mTMS
computer system.

Note that the software installation steps are in `README.md`, while the hardware, operating system, and other pre-requisite steps are in this guide.

## Prerequisites

- Ubuntu 24.04.4 LTS installation media
- Internet connection
- Required hardware components (Edgerouter X, Bittium NeurOne)

## Table of Contents

1. [Router Configuration](#router-configuration)
2. [Windows Configuration](#windows-configuration)
3. [BIOS Configuration](#bios-configuration)
4. [Operating System Installation](#operating-system-installation)
5. [Ubuntu Configuration](#ubuntu-configuration)
6. [Software Installation](#software-installation)
7. [Site-Specific Configurations](#site-specific-configurations)

## Router Configuration

1. Initial Setup
   - Power on the Edgerouter X
   - Connect router's eth0 port to the mTMS computer
   - Disable DHCP on mTMS computer and set:
     - IP address: 192.168.1.2
     - Network mask: 255.255.0.0

2. Router Configuration
   - Access router at 192.168.1.1 via web browser
   - Login with default credentials: ubnt/ubnt
   - Use setup wizard:
     - Set internet port to eth0
     - Reset credentials to ubnt/ubnt
   - Reboot router when prompted

3. Cable Configuration
   - Move computer's ethernet cable to eth1
   - Connect internet cable to eth0
   - Connect Bittium NeurOne to eth3

4. Network Configuration
   - Enable DHCP on mTMS computer
   - Access router at 192.168.1.1
   - Under 'switch' interface:
     - Select Actions -> Config
     - Change IP address to 192.168.200.1/24
     - Save changes

## Operating System Installation

### Install Ubuntu
1. Boot Setup
   - Boot from Ubuntu 24.04.4 LTS installation media
   - Select "Try or Install Ubuntu"
   - If encountering Nouveau driver issues:
     ```
     Press 'e' at boot menu
     Change: linux /casper/vmlinuz ... quiet splash ---
     To: linux /casper/vmlinuz ... quiet splash nomodeset ---
     Press F10 or Ctrl+X to proceed
     ```

2. Installation Configuration
   - Select "Minimal installation"
   - Enable:
     - Download updates while installing
     - Install third-party software
   - Select "Something else" for installation type

3. Partition Setup
   - Create root partition:
     - Size: 150000 MB
     - Type: Primary
     - Location: Beginning
     - File system: Ext4
     - Mount point: /

   - Create home partition:
     - Size: Remaining space
     - Type: Primary
     - Location: Beginning
     - File system: Ext4
     - Mount point: /home

4. User Setup
   - Name: mtms
   - Password: multilocus
   - Enable "Require password to log in"

## Software Installation

### Git Setup
1. Install Git
   ```bash
   sudo apt install git
   ```

2. Install Git LFS
   ```bash
   curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | sudo bash
   sudo apt install git-lfs
   git lfs install
   ```

### Repository Setup
1. Clone Repository
   ```bash
   git clone git@github.com:connect2brain/mtms.git --recurse-submodules
   ```

### MATLAB Installation
1. Download and Install
   - Download R2025b for Linux from mathworks.com
   - Run installation:
     ```bash
     sudo ./install
     ```

2. Required Toolboxes
   - Global Optimization Toolbox
   - Optimization Toolbox
   - ROS Toolbox
   - Signal Processing Toolbox
   - Statistics and Machine Learning Toolbox

3. Configuration
   - Create symbolic links to MATLAB scripts
   - Disable status reports to MathWorks
   - Set keyboard shortcuts to "Windows Default Set"

### mTMS Software Installation
1. Run the installation script:
   ```bash
   scripts/install-mtms [site]
   ```

See the list of available sites in the `sites` directory:
```bash
ls sites
```

After the installation, the computer will be rebooted.

### External EEG software (e.g. NeuroSimo)

`scripts/install-mtms` installs a system service that listens for UDP traffic on one port and sends a copy of each packet to two destinations on localhost:

- **50000** — listen port. Configure the Bittium NeurOne amplifier to send EEG packets to this UDP port.
- **50001** — used by mTMS.
- **50002** — reserved for additional software on the same machine (for example NeuroSimo).

To run NeuroSimo on the mTMS computer:

1. Install it using the instructions in the NeuroSimo repository.
2. Configure NeuroSimo to receive EEG data on **UDP port 50002**.

## Site-Specific Configurations

### MATLAB License Configuration

#### Aalto
- No specific configuration needed
- Ensure computer is on Aalto's internal network

TODO: Note: As of Mar 2026, the MATLAB license server configuration seems to not be working. Please use the local license file for now, and check the IT for help.

#### Tubingen
```bash
sudo cp ~/MATLAB-license/license.lic /usr/local/MATLAB/R2025b/licenses
```

#### Chieti
- Configuration as of Jan 2025 requires manual modification of `ros_entrypoint.sh` in `waveform_approximator`.
TODO: Integrate these changes into the `main` branch for easier deployment.
