# Trigger received on the EEG device.

# Note: Even though Bittium's EEG device has two 'trigger in' ports for a TTL-level trigger,
# one of them is used to synchronize the time between the mTMS device and the EEG device.
#
# Hence, there is only one 'trigger in' port that is published by the EEG bridge, and,
# therefore, no need to distinguish between the trigger ports in the ROS message.

float64 time
