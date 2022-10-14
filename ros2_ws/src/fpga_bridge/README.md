# Permissions
You need have permissions to set task priority and scheduling policy. Modify `/etc/security/limits.conf` and add the following lines:
```
<your username>    -   rtprio    98
<your username>    -   memlock   <limit in kB>

```

Realtime capabilities do not work on Windows.

# Environment variables
If running without docker, ensure that the following environment variables are set.

`FPGA_BRIDGE_BITFILE` should be the name of the bitfile, for example `NiFpga_mtms_0_3_0.lvbitx`
`FPGA_BRIDGE_BITFILE_DIRECTORY` should be the path to the directory where the bitfile is, for example `/home/mtms/workspace/mtms/bitfiles/`
