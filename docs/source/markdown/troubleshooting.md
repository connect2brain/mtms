# Troubleshooting

### MATLAB forgets ROS message types

**Issue:**
MATLAB may sometimes not find previously registered ROS message types, resulting in errors when creating the API object.
For example:

```matlab
>> api = MTMSApi();
...
Unrecognized message type mtms_device_interfaces/DeviceState. Use ros2 msg list to see available types.
...
```

**Possible cause:**
The cause is unknown.

**Solution:**
Run the following script to re-register the message types:

```bash
scripts/register-mtms-matlab
```
