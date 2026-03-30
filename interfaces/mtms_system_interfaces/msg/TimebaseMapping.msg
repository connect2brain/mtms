# Most recent matching pair from sync trigger calibration.
# To convert: mtms_time = (desired_eeg_time - eeg_device_timestamp) + mtms_session_time
bool valid                      # False until first sync trigger after session start
float64 eeg_device_timestamp    # EEG device timestamp of the sync trigger sample
float64 mtms_session_time       # Corresponding mTMS session time (integer seconds)
