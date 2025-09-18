
%%% mTMS toolkit usage examples. %%%
%
%
% General workflow with the mTMS toolkit:
%
% 1. Use generate_pulse_structure() to specify a pulse or sequence of pulses.
%
% 2. Pass the pulse structure to
%       stimulate(), to simply execute the pulse
%       OR
%       run_stimulation_sequence(), to execute a pre-determined sequence of stimuli with error handling and automatic data saving.
%       run_stimulation_trial(), to execute a single stimulus with error handling and data saving.

%% Initialize api

channel_count = 5;  % Change to 6 in Aalto
api = MTMSApi(channel_count);
api.start_device();
api.start_session()

%% Initialize mTMS toolkit

addpath(genpath("/home/mtms/mtms/api/matlab/mTMS_toolkit"))

save_dir = "/home/mtms/projects/mTMS_toolkit/saved/sub000";
mtms_tk = mTMS_toolkit(api,save_dir);

% Use channel 6 instead of 5 (TEMPORARY FIX FOR AALTO)
%mtms_tk.set_channels_in_use([0,1,2,3,5])

%%

%%%%% 1. BASICS %%%%%

%%

% The mTMS device delivers stimuli by loading capacitors and then
% driving the charge to channels, that are connected to the coils of the
% mTMS coil array. Controlling the stimuli happens mainly be specifying the
% capacitor load voltages, as by default, the discharging is done with a
% pre-defined monophasic waveform. Many of the following examples set arbitrary values for
% the load voltages. See Section 6. STIMULATION TARGETING, to learn how these load voltages
% can be defined for different purposes.

%% Run a test pulse to channel 0

% Set load voltages
load_voltages = [100,0,0,0,0];

% Generate a pulse structure from the load_voltages, which automatically
% configures default settings for charge and stimulation timing, waveforms,
% etc.
pulse_structure = mtms_tk.generate_pulse_structure(load_voltages);

% Execute pulse
mtms_tk.stimulate(pulse_structure);

% Discharge load voltage
mtms_tk.full_discharge()

%% Run a test pulse to all channels

load_voltages = [100,100,100,100,100];
pulse_structure = mtms_tk.generate_pulse_structure(load_voltages);
mtms_tk.stimulate(pulse_structure);
mtms_tk.full_discharge()

%% Get EMG readout for a pulse

% When generating the pulse structure, use input argument readout_type, to
% get a readout from the subject after the pulse. An output trigger
% from the mTMS cabinet when a pulse is given is set up with the trigger_out 
% argument by specifying the output port number.

load_voltages = [100,100,100,100,100];
pulse_structure = mtms_tk.generate_pulse_structure(load_voltages,...
                                                   readout_type = 'EMG',...
                                                   trigger_out = 2);
[readout,pulse_ok] = mtms_tk.stimulate(pulse_structure);
mtms_tk.full_discharge()

% Get MEP amplitude.
if pulse_ok
    fprintf("MEP amplitude: %.2e µV\n",readout.amplitude)
end

% It's common that EMG readout fails, if not set up correctly. First, start
% sending the signal from the measurement device. Then, restart the mtms session with:
% api.stop_session(); api.start_session();

%%

%%%%% 2. EXPERIMENTAL BLOCKS %%%%%

%% Repeat a pulse with randomized inter-trial interval

% The pulse timing is set by specifying a 'stim_time' in the pulse structure, and if not specified,
% a default value is used. generate_pulse_structure takes ITI_window as an optional argument, 
% which can be used to randomize stimulation timing by specifying a time range (in seconds)
% relative to the current time.
% 
% For randomized ITI interval, we use the 'run_stimulation_sequence' function,
% which not only automatically handles stimulation timing, 
% but also keeps a constant timing between stimulator charging and stimulation (charging starts mean(ITI_window)/2 before stimulation),
% handles pulse errors, and keeps records of the executed pulses and their readouts.

load_voltages = [100,100,100,100,100];

pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                   ITI_window = [3,4]);

% Repeat the same pulse three times
N_repetitions = 3;
pulse_sequence = mtms_tk.repeat_pulse_structure(pulse_structure, N_repetitions);

result = mtms_tk.run_stimulation_sequence(pulse_sequence);

% Then run_stimulation_sequence automatically saves the 'result' variable
% in the save_dir path with file names "block_1", "block_2", etc.

%% Run different pulses with randomized inter-trial interval

% The timing of between charging and stimulating can be modified. If
% charging time is not important, it's best to charge as soon as possible,
% as sometimes it can take a second or two. Let's specify ITI_window from 4
% to 6 seconds, and start charging 3.5 s before the stimulation.

load_voltage_set = [100,0,0,0,0;
                    0,100,0,0,0;
                    0,0,100,0,0];

ITI_window = [4,6];
charge_to_stim_time = 3.5;
N_pulses = size(load_voltage_set,1);

clear pulse_sequence    % Make sure we have a fresh structure
for i = 1:N_pulses
    pulse_sequence(i) = mtms_tk.generate_pulse_structure(load_voltage_set(i,:),...
                                                         ITI_window = ITI_window,...
                                                         charge_to_stim_time = charge_to_stim_time);
end

% Use the reset_experiment to reset the block counter for the automatic
% file saving. The next filename will then be "block_1".
mtms_tk.reset_experiment()
result = mtms_tk.run_stimulation_sequence(pulse_sequence);

%% Randomized pulse blocks

% Now we'll create four unique stimuli, which will be repeated three times
% in random order. The whole pulse sequence also split in two blocks, which
% can be executed with a break in between.
%
% Each unique pulse is labeled, and the realized pulse sequence (along
% with possible readouts) will be saved in the return value of the
% 'run_stimulation_sequence' function.

load_voltage_set = [2,1,1,1,1;
                    1,2,1,1,1;
                    1,1,2,1,1;
                    1,1,1,2,1]*100;
N_unique_pulses = size(load_voltage_set,1);

ITI_window = [3,4];
charge_to_stim_time = 2.9;  % Charge as soon as possible
N_repetitions = 3;

% Generate pulse sequence for each unique pulse
clear pulse_sequence
for i = 1:N_unique_pulses
    pulse_label = sprintf('pulse_%i',i);
    pulse_sequence(i) = mtms_tk.generate_pulse_structure(load_voltage_set(i,:), ...
                                                         ITI_window = ITI_window, ...
                                                         charge_to_stim_time = charge_to_stim_time, ...
                                                         pulse_label = pulse_label);
end

% Repeat each pulse N times
pulse_sequence = mtms_tk.repeat_pulse_structure(pulse_sequence, N_repetitions);

% Shuffle the order
shuffled_pulse_sequence = mtms_tk.shuffle_pulse_sequence(pulse_sequence);

% Split to blocks
N_blocks = 2;
pulse_sequence_blocks = mtms_tk.split_sequence_to_blocks(shuffled_pulse_sequence, N_blocks);

% Stimulate
fprintf("\nStarting 1st block.\n")
result_block1 = mtms_tk.run_stimulation_sequence(pulse_sequence_blocks{1});

fprintf("\nStarting 2nd block.\n")
result_block2 = mtms_tk.run_stimulation_sequence(pulse_sequence_blocks{2});

%% Stimulation warmup

% Often the first stimulus is suprising and is sometimes discarded in
% data analysis. To avoid this, you can start the stimulation blocks with a
% warmup, that applies pulses in increasing intensity.

warmup_pulse_count = 2;     % Number of pulses in the warmup.
warmup_max_intensity = 0.5; % Warmup starts at 33% MSO intensity until this value.

mtms_tk.warmup(warmup_pulse_count,warmup_max_intensity);

%% 

%%%%% 3. TRIAL-BY-TRIAL STIMULATION %%%%%%

%% 

% Experiments that don't have a pre-determined set of stimuli, such as
% closed-loop experiments, should use the run_stimulation_trial function 
% each time you want to stimulate. This way you have greater control for 
% timing the stimulation, unlike with the run_stimulation_sequence, which starts by 
% restarting the session, randomizes stimulation timing, and discharges the
% device completely in the end. The pulses and their readouts are also
% saved temporarily in the mtms_tk class, for saving to disk later.
%
% Below is example of a closed-loop setup where the load voltages are
% chosen randomly right before each trial.

continue_experiment = 1;    % Flag for ending the experiment
count = 1;
max_pulse_count = 10;

% Reset variables to clear previous trial data
mtms_tk.reset_experiment()

while continue_experiment
    % Decide pulse parameters
    load_voltages = (rand(1,5)-0.5)*200; % Random numbers between -100 and 100
    pulse_label = num2str(count);

    pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                       readout_type = 'EMG', ...
                                                       trigger_out = 2, ...
                                                       pulse_label = pulse_label);
    result = mtms_tk.run_stimulation_trial(pulse_structure);

    readout = result.readout;
    % Decision making...

    % Continue or go next
    if count >= max_pulse_count
        continue_experiment = 0;
    else
        count = count + 1;
    end
end

% Save results to save_dir path.
mtms_tk.save_closed_loop_results()

% Discharge load voltage
mtms_tk.full_discharge()

%%

%%%%% 4. PWM WAVEFORMS AND MULTI-PULSE STIMULI %%%%%%

%%

% The pulse waveforms need special treatment, e.g., in paired-pulse stimulation, where
% the amount of current driven to any single coil changes between the
% delivered stimuli. In these cases, the capacitors are fully charged, and
% pulse waveforms are used to control the release of current in a way that
% we have control over the stimulation intensity. For more info, read: https://doi.org/10.1016/j.brs.2025.04.014.

%% Single-pulse PWM

% Single-pulse PWM may be used in cases where you want to compare the effects 
% between paired- and single-pulse stimuli. 

load_voltages = [0,0,1500,0,1500];
reference_load_voltages = [0,0,800,0,400];

% Define reference waveforms which will be approximated with PWM
reference_waveforms = mtms_tk.get_monophasic_reference_waveforms();

% Generate PWM waveforms for the pulse. (may take ~10 s)
single_pulse_waveforms = mtms_tk.generate_PWM_waveforms(reference_load_voltages, load_voltages, reference_waveforms);

% Generate the pulse structure, and use the waveforms argument to specify the custom waveform.
single_pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                          waveforms = single_pulse_waveforms);

% Execute pulse
mtms_tk.stimulate(single_pulse_structure);

mtms_tk.full_discharge()
% The pulse structure with PWM waveforms can also be used normally with the
% run_stimulation_sequence, or run_stimulation_trial functions.

%% Paired-pulse stimulus

load_voltages = [0,0,1500,0,1500];
reference_load_voltage_set = [0,0,400,0,800;
                              0,0,800,0,400];

% Define reference waveforms which will be approximated with PWM
reference_waveforms(1,:) = mtms_tk.get_monophasic_reference_waveforms();    % First pulse
reference_waveforms(2,:) = mtms_tk.get_monophasic_reference_waveforms();    % Second pulse

% Generate PWM waveforms for the paired pulse. Note that the waveforms for
% the two stimuli must be calculated together because the first stimulus affects
% the available load voltage for the next stimulus.
paired_pulse_waveforms = mtms_tk.generate_PWM_waveforms(reference_load_voltage_set, load_voltages, reference_waveforms);

% Define inter-stimulus interval in seconds
ISI = 10e-3;

% Generate the pulse structure, and use the waveforms argument to specify
% the paired-pulse. Each row of the waveforms argument is automatically 
% intepreted as a multi-pulse sequence.
paired_pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                          waveforms = paired_pulse_waveforms, ...
                                                          ISI = ISI);

% Execute pulse
mtms_tk.stimulate(paired_pulse_structure);

mtms_tk.full_discharge()

%% Quadruple-pulse stimulus

load_voltages = [0,0,1500,0,1500];
reference_load_voltage_set = [0,0,400,0,800;
                              0,0,800,0,400;
                              0,0,400,0,800;
                              0,0,800,0,400];

% Define reference waveforms which will be approximated with PWM
for i = 1:size(reference_load_voltage_set,1)
    reference_waveforms(i,:) = mtms_tk.get_monophasic_reference_waveforms();
end

% Generate PWM waveforms
multi_pulse_waveforms = mtms_tk.generate_PWM_waveforms(reference_load_voltage_set, load_voltages, reference_waveforms);

% Define inter-stimulus interval in seconds
ISIs = [100e-3,100e-3,100e-3];

% Generate the pulse structure
multi_pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                         waveforms = multi_pulse_waveforms, ...
                                                         ISI = ISIs);

% Execute pulse
mtms_tk.stimulate(multi_pulse_structure)

%%

%%%%% 5. BIPHASIC PULSE WAVEFORMS %%%%%

%%

% Default waveforms are monophasic. To create biphasic waveforms, we
% need to specify a piecewise linear function for the electric current, as 
% the mTMS software has no direct implementation for this. It's not
% recommended to customize the following waveform, as each new waveform
% requires careful calibration (bad calibration results in extra heating).
%
% NOTE: The mTMS may give an error "Invalid durations", due to a settings that limits the waveform length.
% Fix this by running "update-settings" from the home directory.

%% Trapezoidal (normal)

% Get piecewise linear waveform structures
reference_waveforms = mtms_tk.get_biphasic_reference_waveforms();

% Convert waveforms to api format.
biphasic_waveforms = mtms_tk.create_waveform_from_struct(reference_waveforms);

% Specify waveform as an argument for the pulse structure and execute
load_voltages = [100,100,100,100,100];
pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                   waveforms = biphasic_waveforms);
mtms_tk.stimulate(pulse_structure);

%% PWM

% We generate single pulse PWM, like in Section 4, but this time use
% get_biphasic_reference_waveforms() instead of
% get_monophasic_reference_waveforms().

load_voltages = [0,0,1500,0,1500];
reference_load_voltages = [0,0,800,0,400];

% Define reference waveforms which will be approximated with PWM
reference_waveforms = mtms_tk.get_biphasic_reference_waveforms();

% Generate PWM waveforms for the single pulse.
single_pulse_waveforms = mtms_tk.generate_PWM_waveforms(reference_load_voltages, load_voltages, reference_waveforms);

% Generate the pulse structure, and use the waveforms argument to specify the custom waveform.
single_pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                          waveforms = single_pulse_waveforms);

% Execute pulse
mtms_tk.stimulate(single_pulse_structure)

%%

%%%%% 6. STIMULATION TARGETING %%%%%

%%

% The princible of stimulation targeting is to determine what load voltages
% must be specified to achieve a desired stimulus, e.g., moving and
% rotating the induced E-field pattern over some cortical region of interest.
%
% There's two main ways: 
% 
% 1. Pre-calculated targeting tables, which use physical measurement of
% E-fields on a spherical shell under the coil array, approximating the
% E-fields on the cortex.
%
% 2. Targeting using a computational E-field model based on MRI segmentation.
%
% Option 1. Is inaccurate, but it's quick and easy to use, and can be
% very useful for example in search of a target that gives the best readout.
%
% Option 2. Is very accurate, and enables better understanding of what parts of the brain 
% are affected by the stimuli, but requires E-field modeling tools.

%% Pre-calculated targeting tables

% Specify E-field displacement relative the the coil center. Use integer
% values between -15 and 15 (mm).
displacement_x = 0;  % mm
displacement_y = 0;  % mm

% Specify E-field rotation relative to coil 'front'. Use integer values
% between 0 and 359 (degrees).
rotation_angle = 0;  % deg

% Specify intensity at the E-field peak.
intensity = 30;  % V/m

load_voltages = mtms_tk.get_target_voltages(displacement_x, displacement_y, rotation_angle, intensity);

% Generate pulse structure
pulse_structure = mtms_tk.generate_pulse_structure(load_voltages);

% Execute pulse
mtms_tk.stimulate(pulse_structure);

% The pulse structure with can also be used normally with the
% run_stimulation_sequence, or run_stimulation_trial functions.

%% Targeting with an E-field model

% This method is currently quite experimental and requires custom software.
% See the open-source GitHub repository: https://github.com/lainem11/E-field_targeting/example_complex_geom.m.
% The example script shows how to optimize the induced E-field pattern for
% stimulation targeting. We will assume you have a saved result from the
% E-field targeting scripts as a file called 'targeting_results.mat'.

% Load targeting results
targeting_results = load('targeting_results.mat');
didt = targeting_results.weights;

% Specify coil file path (not included in the mTMS repository, contact mikael.laine@aalto.fi for assistance)
coil_file = "/home/mtms/mtms/api/matlab/mTMS_toolkit/coils/5coil_aalto_29042025.csv";

% Convert coil weights from the E-field targeting results to load voltages
load_voltages = mtms_tk.didt_to_volts(didt,coil_file);

%%

%%%%% 7. COPY-PASTEABLES %%%%%

%% Basic RMT measurement

% Measure RMT intensity when stimulating at the coil center, direction
% towards the front.

%%% CHANGE HERE %%%
RMT_intensity = 30; % V/m
%%%%%%%%%%%%%%%%%%%

load_voltages = mtms_tk.get_target_voltages(0,0,0,RMT_intensity);

RMT_TS_pulse = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                ITI_window = [3,4], ...
                                                trigger_out = 2, ...
                                                readout_type = 'EMG', ...
                                                pulse_label = pulse_label, ...
                                                charge_to_stim_time = 2.9);

N_repetitions = 20;
RMT_pulse_sequence = mtms_tk.repeat_pulse_structure(RMT_TS_pulse,N_repetitions);

RMT_results = mtms_tk.run_pulse_sequence(RMT_pulse_sequence);

% Check amplitudes
[RMT_MEP_amps,RMT_median] = mtms_tk.extract_amplitude_by_label(RMT_results)
RMT_MEP_array = cell2mat(struct2cell(RMT_MEP_amps));
N_larger_than_50 = sum(RMT_MEP_array > 50);
fprintf("Larger than 50 µV: %i/%i\n",N_larger_than_50,length(RMT_MEP_array))

%%

