# EEG and EMG data in microvolts

float64[] eeg_data
float64[] emg_data
float64 time
# Should 'index' field be here also, as it is in Sample.msg?
bool valid

# The first trigger-in port of the EEG device is used either for:
#   - measuring latency (in case of conventional TMS devices) or
#   - for synchronization pulses (in case of mTMS device).
# In the former case, the trigger is timestamped by the EEG device and it is
# sent to the real-time pipeline. Use this field to indicate that case.
#
# In the latter case, the trigger is managed internally by the EEG bridge,
# hence it does not reach the pipeline, and this is always false.
bool is_latency_measurement_trigger

# The second trigger-in port of the EEG device is used for triggers
# that indicate simultaneous pulses (at least in case of conventional
# TMS devices for measuring the timing error - with mTMS device, it can be
# used more flexibly). Use this field to indicate that case.
#
# TODO: Naming could be clearer. Maybe 'is_stimulation_trigger'? The problem is
#   that it does not necessarily indicate simultaneous stimulation in case of
#   mTMS device.
bool is_trigger

bool is_event
uint16 event_type

PreprocessedSampleMetadata metadata
