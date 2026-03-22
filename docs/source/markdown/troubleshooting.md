# Troubleshooting

### Real-time pipeline cannot keep up with the sample stream

**Issue:**
The real-time pipeline processing rate of a 5 kHz sample stream drops to 2-3 kHz or lower.

**Possible Cause:**
The Neuronavigation computer is in the power-saving mode, which inadvertently slows down the ROS message stream.
The root cause for this is unknown.

**Solution:**
- **Wake from power-saving:** If power-saving mode is active, wake up the computer.
- **Turn off the Neuronavigation computer:** Alternatively, the Neuronavigation computer can be turned off.

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
