targeting_interfaces/ElectricTarget[] targets
float64[] pulse_times_since_trial_start

# Note that 'timing' field can be left undefined when performing experiments
# with the experiment performer. In that case, the experiment performer will
# automatically set the pulse times.
TrialTiming timing

TrialConfig config

TriggerConfig[] triggers

bool analyze_mep
uint8 mep_emg_channel
float64 mep_time_window_start
float64 mep_time_window_end

bool preactivation_check_enabled
float64 preactivation_check_time_window_start
float64 preactivation_check_time_window_end
float64 preactivation_check_voltage_range_limit
