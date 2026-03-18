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

### MATLAB message building crashes in Tuebingen

**Issue:**
Executing scripts/build-mtms-matlab to rebuild ROS message types causes MATLAB to crash. Reinstalling the operating
system does not resolve the problem.

**Possible cause:**
Unknown. There may be compatibility issues affecting the message-building process.

**Solution:**
- **Use pre-built message types:** Utilize the pre-built MATLAB message types available in `api/matlab/matlab_msg_gen.zip`.
Run the registration script:

```bash
scripts/register-mtms-matlab
```

When needed to re-build (i.e., if the message definitions change), do it in Aalto or Chieti, and commit and share the
resulting `.zip` file.

### MEP analyzer does not tolerate dropped samples

**Issue:**
EEG streaming occasionally drops samples due to the UDP protocol, and the MEP analyzer cannot currently handle it.
This prevents successful MEP analysis.

**Solution:**
The long-term solution is to enhance the MEP analyzer to tolerate one or two consecutive dropped samples.

**Additional note:**
If the Bittium NeurOne is configured to "send triggers as channels," EEG timestamps are adjusted by NeurOne
upon receiving a trigger. For example, with a 5 kHz sampling rate, the expected time difference between
consecutive samples is 0.2 ms. However, due to this adjustment mechanism, the interval can vary
between 0.2 ms and 0.3999 ms, potentially causing dropped samples. Hence, changing trigger mode to "send
triggers as packets" may help in preventing samples from dropping.

### Connection-related error messages in logs

**Issue:**
The system logs (displayed by running, e.g., `log-mtms`) show error messages such as:

```
ddsi_udp_conn_write to udp/239.255.0.1:7401 failed with retcode -1
```

**Possible cause:**
ROS2 does not seem to always recover from the computer going to the sleep mode, causing the components to not be
able to communicate with each other, resulting in the error messages such as above.

**Solution:**
Rebooting the computer returns the system into a stable state.
