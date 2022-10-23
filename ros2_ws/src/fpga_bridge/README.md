# Permissions
You need have permissions to set task priority and scheduling policy. Modify `/etc/security/limits.conf` and add the following lines:
```
<your username>    -   rtprio    98
<your username>    -   memlock   <limit in kB>

```

Realtime capabilities do not work on Windows.

# Environment variables
If running without docker, ensure that the following environment variables are set.

`BITFILE` should be the name of the bitfile, for example `NiFpga_mTMS_Tubingen_0.4.1.lvbitx`
`BITFILE_DIRECTORY` should be the path to the directory where the bitfile is, for example `/home/mtms/workspace/mtms/bitfiles/`
`BITFILE_SIGNATURE` should be the signature of the file, for example `B5B3B2DAFB64E53F87C9C0EF36D1D936`. The signatures are stored as text files
in the same directory as the corresponding bitfiles.
