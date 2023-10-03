mep_interfaces/Mep mep

# Because trial can be redone several times, e.g., for MEP failure due to noisy EMG signal, record
# the time when the successful trial was performed.
float64 trial_finish_time

# TODO: Add more information about the failed attempts: e.g., what was the reason for each specific failure?
uint8 num_of_attempts
