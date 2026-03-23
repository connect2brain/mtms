# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

mTMS is an open-source multi-locus transcranial magnetic stimulation (mTMS) platform for flexible brain stimulation research. It is a ROS2-based distributed microservices system that controls FPGA hardware, processes EEG/MEP signals, computes electric fields, and provides a desktop UI for experiment control.

## Architecture

The system has four main layers:

1. **Hardware layer** — `src/mtms_device_bridge/` (C++): Interfaces with a National Instruments FPGA (CompactRIO) via the NI FPGA C API. Provides ~12 ROS2 executables for real-time device control (state, sessions, stimulation triggers, event publishing).

2. **Service layer** — `src/*/` (Python + C++): ~22 containerized ROS2 nodes communicating via CycloneDDS middleware. Key services: `experiment_performer`, `trial_performer`, `targeting`, `waveform_approximator` (MATLAB), `mep_analyzer`, `eeg_bridge`, `mtms_simulator`.

3. **Communication layer** — ROSBridge running on port 9091: bridges ROS2 topics/services to WebSocket for the frontend.

4. **Frontend** — `front/` (React 18 + TypeScript + Electron): Desktop application using ROSLib.js for real-time ROS communication and Chart.js for data visualization.

**ROS2 interfaces** are defined in `interfaces/` (12 packages: `mtms_waveform_interfaces`, `mtms_eeg_interfaces`, `mtms_mep_interfaces`, `mtms_device_interfaces`, `mtms_targeting_interfaces`, `mtms_trial_interfaces`, etc.). These must be built before any service.

**Multi-site support**: `sites/` submodule holds site-specific `.env` configs (device generation, channel count, coil array, FPGA resource IDs, etc.).

## Common Commands

### Build

```bash
# Build ROS workspace (interfaces + realtime_utils first)
source /opt/ros/jazzy/setup.bash
colcon build --base-paths interfaces src/realtime_utils

# Build Docker images
source scripts/build-mtms
# Single container:
CONTAINER_NAME=experiment_performer source scripts/build-mtms

# Build MATLAB ROS interfaces (requires MATLAB + Python 3.10 + GCC-12)
source scripts/build-mtms-matlab

# Build Sphinx documentation
source scripts/build-docs

# Build/package Electron desktop app
bash scripts/build-electron-app
```

### Run

```bash
# Start all services (production)
sudo systemctl start mtms
# or directly:
docker compose up

# Development (includes efield, neuronavigation with X11)
docker compose -f docker-compose.yml -f docker-compose.dev.yml up
```

### Frontend development

```bash
cd front/
npm start              # Dev server on port 3001
npm run electron-dev   # Electron dev mode
npm run build          # Production build
npm run electron-pack  # Package Electron app
```

### Lint / format (frontend)

```bash
cd front/
npm run lint       # ESLint
npm run lint:fix   # ESLint with auto-fix
npm run format     # Prettier
```

### Test

```bash
cd front/
npm test                # React unit tests
npm run cypress:open    # Interactive E2E tests
npm run cypress:run     # Headless E2E tests
```

### Install on a new machine

```bash
scripts/install-mtms [site_name]   # Runs Ansible playbooks for Docker, ROS, samplicator, mTMS setup
```

## Key Files

| File | Purpose |
|------|---------|
| `docker-compose.yml` | Production service definitions |
| `docker-compose.dev.yml` | Dev overrides (efield GPU, neuronavigation) |
| `.env` / `.env.example` | Site-specific device/hardware config |
| `interfaces/` | All ROS2 message/service definitions |
| `src/mtms_device_bridge/CMakeLists.txt` | FPGA hardware bridge executables |
| `config/cyclonedds.xml` | DDS middleware configuration |
| `ansible/install-mtms.yml` | System provisioning playbook |

## Git Submodules

`bitfiles/`, `sites/`, `src/waveform_approximator/waveform_approximator`, `src/efield/efield_libraries`, `src/targeting/data`, `src/neuronavigation/invesalius3` — several are private repos.

## Development Notes

- All services use **host networking** in Docker for low-latency IPC between containers.
- Hardware-facing containers (`mtms_device_bridge`, `efield`) run **privileged** with real-time scheduling.
- When adding a new ROS2 service, model it after existing Python packages (e.g., `src/experiment_performer/`): `package.xml`, `setup.py`, a `Dockerfile`, and an entry in `docker-compose.yml`.
- The ROS workspace `install/` directory is bind-mounted into containers — rebuild with `colcon build` after changing interfaces.
