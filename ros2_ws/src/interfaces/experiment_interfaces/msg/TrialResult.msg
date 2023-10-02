# TODO: Make this more fine-grained, e.g.: Did MEP analysis fail? Did one or several of the
#   stimuli fail? For now, this shall suffice. Also, consider that if trial is redone until it is
#   successful, is this field needed at all.
bool success

mep_interfaces/Mep mep

# Because trial can be redone several times, e.g., for MEP failure due to noisy EMG signal, record
# the time when the successful trial was performed.
float64 time

# TODO: Add more information about the failed attempts: e.g., what was the reason for each specific failure?
uint8 num_of_attempts
