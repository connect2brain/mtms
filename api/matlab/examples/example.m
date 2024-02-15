api = MTMSApi();

api.start_device();
api.start_session();

%%     Single events

%% Charge channel 1 to 20 V.

channel = 1;
target_voltage = 20;
execution_condition = api.execution_conditions.IMMEDIATE;
time = 10.0;
wait_for_completion = true;

api.send_charge(channel, target_voltage, execution_condition, time, wait_for_completion);


%% Allow stimulation before sending a pulse.

api.allow_stimulation(true);

%% Send pulse on channel 1, using the default waveform.

waveform = api.get_default_waveform(channel);
reverse_polarity = false;

api.send_pulse(channel, waveform, execution_condition, time, reverse_polarity, wait_for_completion);


%% Discharge channel 1 completely.

api.send_discharge(channel, 0, execution_condition, time, wait_for_completion);


%% Send trigger out on port 1.

port = 1;
duration_us = 1000;

api.send_trigger_out(port, duration_us, execution_condition, time, wait_for_completion);


%% Send pulse on channel 1 and analyze MEP.

% Use default waveform for the pulse.

waveform = api.get_default_waveform(channel);
reverse_polarity = false;

% Generate a timed pulse.

channel = 1;
execution_condition = api.execution_conditions.TIMED;
time = api.get_time() + 3.0;
wait_for_completion = false;  % Note that this needs to be false so that MEP can be queried for before the pulse is executed.

api.send_pulse(channel, waveform, execution_condition, time, reverse_polarity, wait_for_completion);

% Create a custom waveform.

phases = {'RISING', 'HOLD', 'FALLING'};
durations_in_ticks = {2400, 1200, 1480};

custom_waveform = api.create_waveform(phases, durations_in_ticks);

% Analyze MEP on EMG channel 1, coinciding with the pulse.

% Note that the channel indexing starts from 0.
emg_channel = 0;

mep_start_time = 0.02;  % in ms, after the stimulation pulse
mep_end_time = 0.04;  % in ms
preactivation_check_enabled = true;
preactivation_start_time = -0.02;  % in ms, minus sign indicates that the window starts before the stimulation pulse
preactivation_end_time = -0.01;
preactivation_voltage_range_limit = 70;  % Maximum allowed voltage range inside the time window, in uV.

mep_configuration = api.create_mep_configuration(emg_channel, mep_start_time, mep_end_time, preactivation_check_enabled, preactivation_start_time, preactivation_end_time, preactivation_voltage_range_limit);

[mep, errors] = api.analyze_mep(time, mep_configuration);

amplitude = mep.amplitude;
latency = mep.latency;

%% Targeting

displacement_x = 5;  % mm
displacement_y = 5;  % mm
rotation_angle = 90;  % deg
intensity = 5;  % V/m
algorithm = api.get_targeting_algorithm('least_squares');

[target_voltages, reverse_polarities] = api.get_channel_voltages(displacement_x, displacement_y, rotation_angle, intensity, algorithm);

% Get maximum intensity

maximum_intensity = api.get_maximum_intensity(displacement_x, displacement_y, rotation_angle, algorithm);

% Charge all channels to target voltages.

wait_for_completion = true;
api.send_immediate_charge_or_discharge_to_all_channels(target_voltages, wait_for_completion);

% Send default pulse to all channels.

api.send_immediate_default_pulse_to_all_channels(reverse_polarities, wait_for_completion);


%% Targeting and MEP analysis

displacement_x = 5;  % mm
displacement_y = 5;  % mm
rotation_angle = 90;  % deg
intensity = 5;  % V/m
algorithm = api.get_targeting_algorithm('least_squares');

[target_voltages, reverse_polarities] = api.get_channel_voltages(displacement_x, displacement_y, rotation_angle, intensity, algorithm);

% Charge all channels to target voltages.

wait_for_completion = true;
api.send_immediate_charge_or_discharge_to_all_channels(target_voltages, wait_for_completion);

% Send default pulse to all channels.

wait_for_completion = false;
time = api.get_time() + 3.0;

api.send_timed_default_pulse_to_all_channels(reverse_polarities, time, wait_for_completion);

% Analyze MEP on EMG channel 0, coinciding with the pulse.

% Note that the channel indexing starts from 0.
emg_channel = 0;

mep_start_time = 0.02;  % in ms, after the stimulation pulse
mep_end_time = 0.04;  % in ms
preactivation_check_enabled = true;
preactivation_start_time = -0.02;  % in ms, minus sign indicates that the window starts before the stimulation pulse
preactivation_end_time = -0.01;
preactivation_voltage_range_limit = 70;  % Maximum allowed voltage range inside the time window, in uV.

mep_configuration = api.create_mep_configuration(emg_channel, mep_start_time, mep_end_time, preactivation_check_enabled, preactivation_start_time, preactivation_end_time, preactivation_voltage_range_limit);

[mep, errors] = api.analyze_mep(time, mep_configuration);

amplitude = mep.amplitude;
latency = mep.latency;

%% Restart session

api.stop_session()
api.start_session()


%% Stop device

api.stop_device()
