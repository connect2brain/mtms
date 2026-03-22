# mTMS project

The mTMS software is an open-source software for multi-locus transcranial magnetic stimulation (mTMS), used for
flexible stimulation targeting for research and clinical applications [Koponen et al., 2018]. The software has been
developed within the Connect2Brain project.

Currently, the use of the software requires a custom mTMS device, which is only available to project collaborators.
However, the software can be used as a reference for developing similar systems or for educational purposes.

## Installation

1. **Prerequisites**: Ensure your system meets the following requirements:
   - Supported operating system: Ubuntu 24.04.4 LTS

2. **Installation**: Follow the [Installation guide](docs/source/markdown/installation-guide.md) to prepare your computer and operating system, and install the mTMS software.

### Desktop app setup
The installer builds and installs the Electron-based desktop app automatically.
After installation, you can launch `mTMS panel` from the applications menu
or from the desktop icon.

## Getting started

After installation:

   - Open the "mTMS panel" on your desktop to access the experiment control panel.
   - Explore example scripts in api/python/examples and api/matlab/examples to control the mTMS device.
   - Detailed API documentation is available via the "mTMS Documentation" desktop shortcut.

After opening the mTMS panel, ensure that it reports that the mTMS device is powered on
and started. If not, turn it on by pressing the power button on the device.

## Documentation

Site-specific configurations and known issues are detailed in the [Known differences](docs/source/markdown/known-differences.md) document.

General troubleshooting guidance is available in [Troubleshooting](docs/source/markdown/troubleshooting.md).

## License

This software is licensed under the GPL v3. See the [LICENSE](LICENSE) file for more information.

### External libraries and dependencies

This software depends on several external libraries and software, which are connected to this repository via Git submodules. Some of these
are proprietary or have their own licenses. In addition, certain repositories are private and require appropriate permissions for access.
Here are a few of the external repositories and their locations in the directory structure:

- [InVesalius3](https://github.com/invesalius/invesalius3): Located at `src/neuronavigation/invesalius3`
- [E-field library](https://github.com/connect2brain/e-field): Used for electric field estimation, located at `src/efield/src` (private repository)
- [Waveform Approximator](https://github.com/connect2brain/waveform-approximator): For approximating pulse waveforms, located at `src/waveform_approximator` (private repository)

For a complete list, see the .gitmodules file. Refer to each repository’s root for license and authorship details.

Access to private repositories may require contacting the maintainers or obtaining project-specific permissions.

## References

- Koponen, Lari M., Nieminen, Jaakko O., & Ilmoniemi, Risto J. (2018). Multi-locus transcranial magnetic stimulation—theory and implementation. *Brain Stimulation, 11*(4), 849–855. Elsevier. https://doi.org/10.1016/j.brs.2018.03.014
