# ============================================================================
# Core data
# ============================================================================
float64[] eeg                   # Length = num_eeg_channels, in microvolts
float64[] emg                   # Length = num_emg_channels, in microvolts
float64 time                    # Sample time, seconds since recording start
uint64 sample_index             # Sample count (0-based)

# ============================================================================
# Hardware triggers
# ============================================================================
bool trigger_a
bool trigger_b

# ============================================================================
# Metadata
# ============================================================================
uint64 system_time_eeg_bridge_published
