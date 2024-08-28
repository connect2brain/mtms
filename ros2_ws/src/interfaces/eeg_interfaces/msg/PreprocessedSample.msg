# EEG and EMG data in microvolts

float64[] eeg_data
float64[] emg_data
float64 time
# Should 'index' field be here also, as it is in Sample.msg?
bool valid
bool trigger

PreprocessedSampleMetadata metadata
