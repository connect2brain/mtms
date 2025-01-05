# PREEMPT_RT Kernel Installation Guide

This guide provides instructions for installing the PREEMPT_RT kernel patch for kernel version 6.6 (as of January 2024).

## Prerequisites

- Linux system with administrative privileges
- Internet connection
- Basic knowledge of terminal commands

## Installation Steps

### 1. Repository Setup

1. Clone the Docker Realtime repository:
   ```bash
   git clone https://github.com/2b-t/docker-realtime
   ```

### 2. Installation Process

1. Navigate to the source directory and create a temporary folder:
   ```bash
   cd src
   mkdir tmp
   cd tmp
   ```

2. Run the installation script:
   ```bash
   ./../install_debian_preemptrt
   ```

3. During installation:
   - Select the `sid` version when prompted
   - Follow the installation instructions provided by the script

### 3. Package Installation

1. Download the required package:
   - Package name: `linux-image-6.6.11-rt-amd64`
   - Source: [Debian Packages Repository](https://packages.debian.org/sid/linux-image-6.6.11-rt-amd64)

2. Install the downloaded package:
   ```bash
   sudo dpkg -i linux-image-6.6.11-rt-amd64_6.6.11-1_amd64.deb
   ```

### 4. Post-Installation

1. Re-run the installation script:
   ```bash
   ./../install_debian_preemptrt
   ```

2. Reboot your system to apply changes:
   ```bash
   sudo reboot
   ```

## Important Notes

- This kernel patch does not support automatic updates
- Manual upgrading is required to maintain the latest version
- Installation was verified with kernel version 6.6 (Matilda's laptop)

## Additional Resources

For more detailed information, refer to:
- [Docker Realtime Repository](https://github.com/2b-t/docker-realtime)
- [PreemptRT Documentation](https://github.com/2b-t/docker-realtime/blob/main/doc/PreemptRt.md)
