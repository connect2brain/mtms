# mTMS Real-time Computer Installation Guide

This guide provides step-by-step instructions for installing and configuring an mTMS real-time computer system. Some steps are specific to the lab computer installation, while others (marked with *) are required for both lab computers and mobile laptops.

Note that the software installation steps are in `README.md`, while the hardware, operating system, and other pre-requisite steps are
in this guide.

## Prerequisites

- Windows 11 pre-installed on the computer
- Ubuntu 22.04.3 LTS installation media
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
   - Connect router's eth0 port to the real-time computer
   - On Windows, disable DHCP and set:
     - IP: 192.168.1.2
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
   - Enable DHCP on real-time computer
   - Access router at 192.168.1.1
   - Under 'switch' interface:
     - Select Actions -> Config
     - Change IP address to 192.168.200.1/24
     - Save changes

## Windows Configuration

### Disable BitLocker
1. Open Windows search
2. Search for "Data encryption"
3. Disable encryption

## BIOS Configuration

1. Update BIOS
   - Open HP Support Assistant
   - Navigate to Device Support -> Software & Drivers
   - Install HP Consumer Desktop PC BIOS Update

2. Configure BIOS Settings
   - Access BIOS during boot (usually Esc key on HP Envy)
   - Change language to English
   - Disable secure boot

## Operating System Installation*

### Prepare for Dual Boot
1. In Windows:
   - Open Disk Management
   - Shrink main partition to 200 GB
   - Leave remaining space unallocated

### Install Ubuntu
1. Boot Setup
   - Boot from Ubuntu 22.04.3 LTS installation media
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
     - Size: 120000 MB
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

   - Set boot loader:
     - Device: /dev/nvme0n1p1 (Windows Boot Manager)

4. User Setup
   - Name: mtms
   - Password: multilocus
   - Enable "Require password to log in"

## Ubuntu Configuration*

1. Install Ubuntu Pro
   - Complete initial Ubuntu Pro setup
   - Create personal Ubuntu Pro account

2. Install Real-time Kernel
   ```bash
   sudo pro enable realtime-kernel
   ```

## Software Installation*

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
1. Generate SSH Key
   ```bash
   ssh-keygen -o
   ```

2. Add SSH Key to GitHub
   - Copy contents of ~/.ssh/id_rsa.pub
   - Add to GitHub under Settings -> SSH and GPG keys

3. Clone Repository
   ```bash
   git clone git@github.com:connect2brain/mtms.git --recurse-submodules
   ```

### MATLAB Installation
1. Download and Install
   - Download R2024b for Linux from mathworks.com
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

## Site-Specific Configurations

### MATLAB Docker License

#### Aalto
- No specific configuration needed
- Ensure computer is on Aalto's internal network

#### Tubingen
```bash
sudo cp ~/MATLAB-license/license_mtms-tubingen_556154_R2024b.lic /usr/local/MATLAB/R2024b/licenses
```

#### Chieti
- Configuration TBD (As of Sep 2024)

### InVesalius Installation (Chieti Only)
1. Install dependencies as per [Wiki](https://github.com/invesalius/invesalius3/wiki)
2. Build Cython modules in `invesalius_ros/ros2_ws/src/neuronavigation/neuronavigation/invesalius3`
3. Verify installation by running:
   ```bash
   python3 app.py
   ```
