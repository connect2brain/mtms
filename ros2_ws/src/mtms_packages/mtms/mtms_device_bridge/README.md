# mTMS device bridge

Communication bridge between the mTMS device and the ROS 2 system. The bridge is implemented in C++ and uses the [NiFpga API](https://www.ni.com/en-us/support/documentation/supplemental/18/ni-fpga-interface-c-api.html) to communicate with the FPGA. The bridge is responsible for:

- Loading the bitfile to the FPGA.
- Reading and writing data to the FPGA.
- Publishing and subscribing to ROS 2 topics.

## Usage

### Running with docker

As of now, the bridge only works without docker. The docker image is still in development (see Dockerfile).

### Running without docker

To run the bridge without docker, run the script in the `scripts` directory:
```bash
scripts/start-mtms-bridge
```

## Environment variables
When run without docker, ensure that the following environment variables are set. They are used to configure the bridge, and are automatically set when run using the `start-mtms-bridge` script using the `.env` file:

- `BITFILE` is the name of the bitfile, for example `NiFpga_mTMS_generation_1_0.5.12.lvbitx`.
- `BITFILE_DIRECTORY` is the path to the directory in which the bitfile is, e.g., `/home/mtms/mtms/bitfiles/`.
- `BITFILE_SIGNATURE` is the signature of the file, e.g., `B5B3B2DAFB64E53F87C9C0EF36D1D936`. The signatures are stored as text files in the same directory as the corresponding bitfiles.
- `RESOURCE` is the name of the FPGA resource, e.g., `PXI1Slot3`.
