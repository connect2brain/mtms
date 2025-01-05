# Known Differences Between Installations

This document outlines the specific differences and configurations of mTMS computers at different installation sites.

## Chieti

- **Memory Configuration:**
  - The mTMS computer has **32 GB of RAM**, whereas the computers in Tübingen and Aalto have **64 GB**.
  - To compensate, an additional **32 GB swap file** was set up.

- **Swap File Setup:**
  - The swap file was created using the following commands:
    ```bash
    sudo fallocate -l 32G /home/swapfile
    sudo chmod 600 /home/swapfile
    sudo mkswap /home/swapfile
    sudo swapon /home/swapfile
    ```
  - To make the swap file permanent, the following line was added to `/etc/fstab`:
    ```
    /home/swapfile none swap sw 0 0
    ```

## Tübingen

- **Disk Space Allocation:**
  - The root partition (`/`) has only **100 GB** available.
  - The home partition (`/home`) has only **500 GB** available.
  - Efforts are underway to increase the size of the root partition (as of September 2024).

- **MATLAB Startup Delay:**
  - MATLAB takes approximately **3 minutes** to start.
  - Sample log output:
    ```
    mtms-waveform_approximator-1 | Registering ROS messages in MATLAB...
    mtms-waveform_approximator-1 | Starting ROS node in MATLAB...
    mtms-waveform_approximator-1 |
    mtms-waveform_approximator-1 |                             < M A T L A B (R) >
    mtms-waveform_approximator-1 |                   Copyright 1984-2024 The MathWorks, Inc.
    mtms-waveform_approximator-1 |              R2024a Update 5 (24.1.0.2653294) 64-bit (glnxa64)
    mtms-waveform_approximator-1 |                                June 24, 2024
    ```

- **Kernel Version Issues:**
  - Attempted to run kernel versions `6.5.0-27-generic` and `6.5.0-27-lowlatency`, as officially required by CUDA.
  - Both versions encountered issues during startup.
  - Kernel `6.5.0-27` displayed startup errors (details not provided).
