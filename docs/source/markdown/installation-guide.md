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
2. [BIOS Configuration](#bios-configuration)
3. [Operating System Installation](#operating-system-installation)
4. [Ubuntu Configuration](#ubuntu-configuration)
5. [Software Installation](#software-installation)
6. [Site-Specific Configurations](#site-specific-configurations)

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
     - Size: 250000 MB
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

## Ubuntu Configuration

> **Warning:** Do not run `apt upgrade` or `apt dist-upgrade` after installation. The National Instruments drivers are built against a specific kernel version, and upgrading the kernel will break them.

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
   git clone https://github.com/connect2brain/mtms.git --recurse-submodules
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

Installation is split into two phases, each followed by a reboot.

**Phase 1: system stack**

From the repository root run:

```bash
bash scripts/install-mtms-system
```

After this phase, the computer needs to be rebooted.

**Phase 2: mTMS application**

From the repository root, run:

```bash
bash scripts/install-mtms [site]
```

`[site]` selects site-specific `.env` from `sites/<site>/.env`. List available sites:

```bash
ls sites
```

After this phase, the computer needs to be rebooted again.

### Setting up the neuronavigation computer

TBD: The modern way of setting it up should be documented here.

An important note: "Wake on LAN" needs to be disabled on the neuronavigation computer. Otherwise, the network
interface card won't respond properly to multicast traffic sent by the mTMS computer, causing EEG streaming
to drop samples. Here are the steps to disable "Wake on LAN":

1. In BIOS, disable "Wake on LAN".
2. In Windows 11, go to Device Manager → Network adapter → Properties → Power Management, and uncheck "Allow the computer to turn off this device to save power."

### External EEG software (e.g. NeuroSimo)

During mTMS installation, a system service is installed that listens for UDP traffic on one port and sends a copy of each packet to two destinations on localhost:

- **50000** — listen port. Configure the Bittium NeurOne amplifier to send EEG packets to this UDP port.
- **50001** — used by mTMS.
- **50002** — reserved for additional software on the same machine (for example NeuroSimo).
- **50003** — reserved for user applications, e.g., custom MATLAB scripts that need the EEG stream.

To run NeuroSimo on the mTMS computer:

1. Install it using the instructions in the NeuroSimo repository.
2. Configure `.env` in repository root to have `ROS_AUTOMATIC_DISCOVERY_RANGE=SUBNET` instead of the default `LOCALHOST`, so that NeuroSimo can communicate directly with the neuronavigation computer.
3. Configure NeuroSimo to receive EEG data on **UDP port 50002** (under "File" -> "Settings").
4. Configure NeuroSimo to **disable LabJack** (under "File" -> "Settings" -> "LabJack").

### EEG device configuration

Use the following settings for the Bittium NeurOne, found in the protocol settings under "Real-Time Out" -> "Settings":

## Main settings

- **Packet Frequency (Hz):** [needs to match the sampling frequency of the protocol, e.g. 5000 Hz]
- **Target IP Address:** `192.168.200.221`
- **Target UDP Port:** `50000`

## Trigger Sending Mode

Select:

- `(x) Send Triggers as a Channel`
- `( ) Send Triggers as Packets`
- `( ) Do Not Send Triggers`

## Additional Options

- `[x] Send Packets MeasurementStart and MeasurementEnd`
- `[ ] Send HardwareState Packets`
  - `(This includes ClockSourceState Packets)`
- `[ ] Trigger delivery on rising edge`
  - `(trigger sample channel)`

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
