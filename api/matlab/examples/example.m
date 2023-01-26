api = MTMSApi();

api.start_device();

api.stop_experiment();
api.start_experiment();

%%     Single events

%% Charge channel 1 to 20 V.

channel = 1;
target_voltage = 20;
execution_condition = api.execution_conditions.IMMEDIATE;
time = 10.0;
wait_for_completion = true;

api.send_charge(channel, target_voltage, execution_condition, time, wait_for_completion);


%% Send pulse on channel 1, using the default waveform.

waveform = api.get_default_waveform(channel);
reverse_polarity = false;

api.send_pulse(channel, waveform, execution_condition, time, reverse_polarity, wait_for_completion);


%% Discharge channel 1 completely.

api.send_discharge(channel, 0, execution_condition, time, wait_for_completion);


%% Send trigger on port 1.

port = 1;
duration_us = 1000;

api.send_signal_out(port, duration_us, execution_condition, time, wait_for_completion);


%% Send pulse on channel 1 and analyze MEP.

api.stop_experiment();
api.start_experiment();

% Use default waveform for the pulse.

waveform = api.get_default_waveform(channel);
reverse_polarity = false;

% Generate a timed pulse.

channel = 1;
execution_condition = api.execution_conditions.TIMED;
time = 5.0;
wait_for_completion = false;  % Note that this needs to be false.

api.send_pulse(channel, waveform, execution_condition, time, reverse_polarity, wait_for_completion);

% Analyze MEP on EMG channel 1, coinciding with the pulse.

emg_channel = 1;
[amplitude, latency] = api.analyze_mep(emg_channel, time);


%% Targeting

displacement_x = 5;  % mm
displacement_y = 5;  % mm
rotation_angle = 90;  % deg
intensity = 20;  % V/m

[target_voltages, reverse_polarities] = api.get_channel_voltages(displacement_x, displacement_y, rotation_angle, intensity);

% Charge all channels to target voltages.

wait_for_completion = true;

api.send_immediate_charge_or_discharge_to_all_channels(target_voltages, wait_for_completion);

% Send default pulse to all channels.

api.send_immediate_default_pulse_to_all_channels(reverse_polarities, wait_for_completion);


%% Targeting and MEP analysis

api.stop_experiment();
api.start_experiment();

displacement_x = 5;  % mm
displacement_y = 5;  % mm
rotation_angle = 90;  % deg
intensity = 20;  % V/m

[target_voltages, reverse_polarities] = api.get_channel_voltages(displacement_x, displacement_y, rotation_angle, intensity);

% Charge all channels to target voltages.

wait_for_completion = true;

api.send_immediate_charge_or_discharge_to_all_channels(target_voltages, wait_for_completion);

% Send default pulse to all channels.

wait_for_completion = false;
time = api.get_time() + 3.0;

api.send_timed_default_pulse_to_all_channels(reverse_polarities, time, wait_for_completion);

% Analyze MEP on EMG channel 1, coinciding with the pulse.

emg_channel = 1;
[amplitude, latency] = api.analyze_mep(emg_channel, time);


%% Restart experiment

api.stop_experiment()
api.start_experiment()


%% Stop device

api.stop_device()
