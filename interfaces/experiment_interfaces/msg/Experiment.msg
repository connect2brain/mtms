string experiment_name
string subject_name

trial_interfaces/Trial[] trials

float64 intertrial_interval_min
float64 intertrial_interval_max
float64 intertrial_interval_tolerance

bool randomize_trials

# If true, wait for pedal press before performing each trial in
# the experiment. In that case, ignore the intertrial interval fields.
bool wait_for_pedal_press

bool autopause
uint16 autopause_interval

# MEP analysis configuration (applies to all trials in the experiment)
bool analyze_mep
uint8 mep_emg_channel
float64 mep_time_window_start
float64 mep_time_window_end

bool preactivation_check_enabled
float64 preactivation_check_time_window_start
float64 preactivation_check_time_window_end
float64 preactivation_check_voltage_range_limit
