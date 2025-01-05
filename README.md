# mTMS project

The mTMS software is an open-source software for multi-locus transcranial magnetic stimulation (mTMS), used for
flexible stimulation targeting for research and clinical applications [Koponen et al., 2018]. The software has been
developed within the Connect2Brain project.

Currently, the use of the software requires a custom mTMS device, which is only available to project collaborators.
However, the software can be used as a reference for developing similar systems or for educational purposes.

## Installation

1. **Prerequisites**: Ensure your system meets the following requirements:
   - Supported operating system: Ubuntu 22.04.3 LTS

2. **System setup**: Follow the [Installation guide](docs/source/markdown/installation-guide.md) to prepare your computer, operating system, and external software.

3. **Install mTMS software**: Run the installation script:
   ```bash
   source scripts/install.sh [site]

Replace [site] with a valid site name from the sites directory:

```bash
ls sites
```

The installation script will install the mTMS software and its dependencies, including the ROS 2 workspace.

### Web UI setup
To create a desktop link to the mTMS panel:
   - Open Chrome and navigate to http://localhost:3001
   - Click "Install mTMS panel"
   - Enable launching for desktop shortcut

## Getting started

After installation:

   - Open the "mTMS panel" on your desktop to access the experiment control panel.
   - Explore example scripts in api/python/examples and api/matlab/examples to control the mTMS device.
   - Detailed API documentation is available via the "mTMS Documentation" desktop shortcut.

After opening the mTMS panel, ensure that it reports that the mTMS device is powered on
and started. If not, first turn it on by pressing the power button on the device.
After that, press "Start device" on the front-page of the panel and wait for
the device to start.

Ensure also that:
   - A BNC cable is connected from the 'sync' port of the mTMS device to 'Port A in' of the EEG/EMG device.
   - Check that the EEG/EMG device is connected to the real-time computer with an Ethernet cable via the router.

## Known differences between installations

Site-specific configurations and known issues are detailed in the [Known differences](docs/source/markdown/known-differences.md) document.

## Guide for installing PREEMPT_RT kernel patch

For instructions on installing the PREEMPT_RT patch for recent Linux kernels, see the [PREEMPT_RT installation guide](docs/source/markdown/preempt-rt-guide.md).

## License

This software is licensed under the GPL v3. See the [LICENSE](LICENSE) file for more information.

### External libraries and dependencies

This software depends on several external libraries and software, which are connected to this repository via Git submodules. Some of these
are proprietary or have their own licenses. In addition, certain repositories are private and require appropriate permissions for access.
Here are a few of the external repositories and their locations in the directory structure:

- [InVesalius3](https://github.com/invesalius/invesalius3): Located at `ros2_ws/src/mtms_packages/neuronavigation/neuronavigation/invesalius3`
- [E-field library](https://github.com/connect2brain/e-field): Used for electric field estimation, located at `ros2_ws/src/mtms_packages/targeting/efield/src` (private repository)
- [Waveform Approximator](https://github.com/connect2brain/waveform-approximator): For approximating pulse waveforms, located at `ros2_ws/src/mtms_packages/targeting/waveform_approximator` (private repository)

For a complete list, see the .gitmodules file. Refer to each repository’s root for license and authorship details.

Access to private repositories may require contacting the maintainers or obtaining project-specific permissions.

## References

- Koponen, Lari M., Nieminen, Jaakko O., & Ilmoniemi, Risto J. (2018). Multi-locus transcranial magnetic stimulation—theory and implementation. *Brain Stimulation, 11*(4), 849–855. Elsevier. https://doi.org/10.1016/j.brs.2018.03.014
