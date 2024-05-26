float64 desired_start_time

# If true, allow performing the trial at any time after 'desired_start_time'. If false and it is past 'desired_start_time',
# fail the trial.
bool allow_late

# If true, wait for trigger (a pedal press or an electric trigger signal) before performing the trial.
#
# Note: If true, 'desired_start_time' and 'allow_late' fields are ignored.
bool wait_for_trigger
