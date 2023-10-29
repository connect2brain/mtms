float64 time

# If true, allow performing the trial at any time after 'time'. If false and 'time' has passed, fail the trial.
bool allow_late

# If true, wait for trigger (a pedal press or an electric trigger signal) before performing the trial.
#
# Note: If true, 'time' and 'allow_late' fields are ignored.
bool wait_for_trigger
