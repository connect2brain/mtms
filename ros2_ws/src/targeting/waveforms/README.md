# Permissions
You need have permissions to set task priority and scheduling policy. Modify `/etc/security/limits.conf` and add the following lines:
```
<your username>    -   rtprio    98
<your username>    -   memlock   <limit in kB>

```

Realtime capabilities do not work on Windows.
