
%%% mTMS toolkit usage examples. %%%
%
%
% General workflow with the mTMS toolkit:
%
% 1. Use generate_pulse_structure() to specify a pulse. Multiple pulse structures are stacked
% in an array to run a pulse sequence.
%
% 2. Pass the pulse structure to
%       stimulate(), to simply execute the pulse
%       OR
%       run_stimulation_sequence(), to execute a pre-determined sequence of stimuli with error handling and automatic data saving.
%       run_stimulation_trial(), to execute a single stimulus with error handling and data saving.

%% Table of Contents

% Initalize mTMS api
% Initialize mTMS toolkit

% 1.1. Run simple pulse to a single channel
% 1.2. Using coil-specific waveform calibration
% 1.3. Run pulse to all channels
% 1.4. Get EMG readouts

% 2.1. Repeat pulse with randomized inter-trial interval
% 2.2. Run different pulses with randomized inter-trial interval
% 2.3. Randomized pulse blocks
% 2.4. Stimulation warmup

% 3.1. Measurement loop
% 3.2. Handling pulse strain

% 4.1. Single-pulse PWM
% 4.2. Paired-pulse PWM
% 4.3. Quadruple-pulse stimulus

% 5.1. Pre-calculated targeting tables
% 5.2. Targeting with an E-field model

% 6.1. Basic RMT measurement
%% Initialize mTMS api

channel_count = 5;
api = MTMSApi(channel_count);
api.start_device()
api.start_session()

%% Initialize mTMS toolkit

addpath(genpath("/home/mtms/mtms/api/matlab/mTMS_toolkit"))

% Set directory for automatic saving. Set to [] no disable saving.
save_dir = [];

mtms_tk = mTMS_toolkit(save_dir);
mtms_tk.add_api(api);

% Note: Earlier versions included additional coil file for initializing the
% toolkit, but this is now optional.

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

%% 1.1. Run simple pulse to a single channel

% Load the first channel with 100 Volts, other channels to 0.
load_voltages = [100,0,0,0,0];

% Generate a pulse structure from the load_voltages, which automatically
% configures default settings for charge and stimulation timing, waveforms,
% etc.

pulse_structure = mtms_tk.generate_pulse_structure(load_voltages);

% Execute pulse
mtms_tk.stimulate(pulse_structure);

% Discharge load voltage.
mtms_tk.full_discharge()

% NOTE: You should see a warning in the terminal: "Warning: Waveform
% calibration not found. Using default ramp-down timing." This happens
% because pulse is given with a default monophasic waveform that is not
% calibrated for the specific coil. Calibrated waveform reduces stray
% current after the pulse which reduces the heat generation slightly.

%% 1.2. Using coil-specific waveform calibration

% IMPORTANT! It's highly recommended to include the approximator in the
% mTMS toolkit, as it enables safety features to prevent coil damage.

% Define calibration file for the coil in use
calibration_filepath = "/home/mtms/mtms/src/waveform_approximator/waveform_approximator/data/tubingen_mk2/calibration.mat";

% The calibration file contains data that is used in the monophasic
% waveform calibration, pulse strain estimation, PWM approximations, and
% voltage-to-amperes unit conversions. 

% Add waveform approximator to the toolkit
mtms_tk.add_approximator(calibration_filepath)

%% 1.3. Run pulse to all channels

load_voltages = [100,100,100,100,100];
pulse_structure = mtms_tk.generate_pulse_structure(load_voltages);
mtms_tk.stimulate(pulse_structure);
mtms_tk.full_discharge()

%% 1.4. Get EMG readouts

% When generating the pulse structure, use input argument readout_type, to
% get a readout from the subject after the pulse. An output trigger
% from the mTMS cabinet when a pulse is given is set up with the trigger_out 
% argument by specifying the output port number.

load_voltages = [100,100,100,100,100];

pulse_structure = mtms_tk.generate_pulse_structure(load_voltages,...
                                                   readout_type = 'EMG',...
                                                   trigger_out = 2);
[readout,pulse_diagnostic] = mtms_tk.stimulate(pulse_structure);
mtms_tk.full_discharge()

% Get MEP amplitude.
if pulse_diagnostic.ok_flag
    fprintf("MEP amplitude: %.2e µV\n",readout.amplitude)
end

% It's common that EMG readout fails, if not set up correctly. First, start
% sending the signal from the measurement device. Then, restart the mtms session with:
% api.stop_session(); api.start_session();

%%

%%%%% 2. EXPERIMENTAL BLOCKS %%%%%

%% 2.1. Repeat pulse with randomized inter-trial interval

% The pulse timing is set by specifying a 'stim_time' in the pulse structure, and if not specified,
% a default value is used. generate_pulse_structure takes ITI_window as an optional argument, 
% which can be used to randomize stimulation timing by specifying a time range in seconds.
% 
% For randomized ITI interval, we use the 'run_stimulation_sequence' function,
% which not only automatically handles stimulation timing, but also keeps a constant timing between 
% stimulator charging and stimulation (charging starts mean(ITI_window)/2 before stimulation),
% handles pulse errors, and keeps records of the executed pulses and their readouts.

load_voltages = [100,100,100,100,100];

pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                   ITI_window = [3,4]);

% Repeat the same pulse three times
N_repetitions = 3;
pulse_sequence = mtms_tk.repeat_pulse_structure(pulse_structure, N_repetitions);

result = mtms_tk.run_stimulation_sequence(pulse_sequence);

% Then run_stimulation_sequence automatically saves the 'result' variable
% in the save_dir path with file names "block_1", "block_2", etc. The block
% numbering can be reset with "mtms_tk.reset_experiment()", but beware that
% any new saved blocks will overwrite the previous files.

%% 2.2. Run different pulses with randomized inter-trial interval

% The timing of between charging and stimulating can be modified. If
% charging time is not important, it's best to charge as soon as possible,
% as sometimes it can take a second or two. Let's specify ITI_window from 4
% to 6 seconds, and start charging 3.9 s before the stimulation.

load_voltage_set = [100,0,0,0,0;
                    0,100,0,0,0;
                    0,0,100,0,0];

ITI_window = [4,6];
charge_to_stim_time = 3.9;
N_pulses = size(load_voltage_set,1);

clear pulse_sequence
for i = 1:N_pulses
    pulse_sequence(i) = mtms_tk.generate_pulse_structure(load_voltage_set(i,:),...
                                                         ITI_window = ITI_window,...
                                                         charge_to_stim_time = charge_to_stim_time);
end

result = mtms_tk.run_stimulation_sequence(pulse_sequence);

% NOTE: This sequence may display prints saying:
% "[Done] Pulse  Event ID: XXX, Status: Late", followed with: 
% "Error in pulse execution".
% This happens because changing the load voltages took longer than the
% specified charge_to_stim_time. This is a known limitation of the system, 
% but it happens less when channels are not fully discharged  to zero between pulses. 
% The mTMS toolkit automatically retries the failed pulses, for a maximum of three times. 

%% 2.3. Randomized pulse blocks

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

%% 2.4. Stimulation warmup

% Often the first stimulus is abtrupt and is sometimes discarded in
% data analysis. To avoid this, you can start the stimulation blocks with a
% warmup, that applies pulses in increasing intensity.

warmup_pulse_count = 3;     % Number of pulses in the warmup.
warmup_max_intensity = 0.75; % Warmup starts at 33% MSO intensity until this value.

mtms_tk.warmup(warmup_pulse_count,warmup_max_intensity);

mtms_tk.full_discharge()

%% 

%%%%% 3. TRIAL-BY-TRIAL STIMULATION %%%%%%

%% 3.1. Measurement loop

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

%% 3.2. Handling pulse strain

% When a waveform approximator is added to the mTMS toolkit, pulse strain
% is estimated inside the "generate_pulse_structure" function. If a pulse
% would exceed the strain limit, it gives an error to prevent executing
% such pulses. It is possible to automatically scale the pulse strength to
% the allowed limit, by setting the "clamp_strain" argument to true.

% Try to create a pulse above the allowed limit (gives an error)
pulse_structure = mtms_tk.generate_pulse_structure([1500,1500,1500,1500,1500]);

%%
% Use the clamp strain argument (automatically scales the voltages down)
pulse_structure = mtms_tk.generate_pulse_structure([1500,1500,1500,1500,1500], ...
                                                    clamp_strain=true);

%%

%%%%% 4. PWM WAVEFORMS AND MULTI-PULSE STIMULI %%%%%%

%%

% The pulse waveforms need special treatment, e.g., in paired-pulse stimulation, where
% the amount of current driven to any single coil changes between the
% delivered stimuli. In these cases, the capacitors are fully charged, and
% pulse waveforms are used to control the release of current in a way that
% we have control over the stimulation intensity. For more info, read: https://doi.org/10.1016/j.brs.2025.04.014.

%% 4.1. Single-pulse PWM

% Single-pulse PWM may be used in cases where you want to compare the effects 
% between paired- and single-pulse stimuli. 

load_voltages = [0,1500,0,1500,0];
reference_load_voltages = [0,500,0,500,0];

% Define reference waveforms which will be approximated with PWM
reference_waveforms = mtms_tk.get_monophasic_reference_waveforms();

% Generate PWM waveforms for the pulse.
single_pulse_waveforms = mtms_tk.generate_PWM_waveforms(reference_load_voltages, load_voltages, reference_waveforms);

% Generate the pulse structure, and use the waveforms argument to specify the custom waveform.
single_pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
                                                          waveforms = single_pulse_waveforms);

% Execute pulse
mtms_tk.stimulate(single_pulse_structure);

mtms_tk.full_discharge()
% The pulse structure with PWM waveforms can also be used normally with the
% run_stimulation_sequence, or run_stimulation_trial functions.

%% 4.2. Paired-pulse PWM

load_voltages = [0,1500,0,1500,0];
reference_load_voltage_set = [0,400,0,400,0;
                              0,800,0,400,0];

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

%% 4.3. Quadruple-pulse stimulus

load_voltages = [1500,1500,1500,1500,1500];
reference_load_voltage_set = [400,200,200,200,200;
                              200,400,200,200,200;
                              200,200,400,200,200;
                              200,200,200,400,200];

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

mtms_tk.full_discharge()

%%

%%%%% 5. STIMULATION TARGETING %%%%%

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

%% 5.1. Pre-calculated targeting tables

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

mtms_tk.full_discharge()

% The pulse structure with can also be used normally with the
% run_stimulation_sequence, or run_stimulation_trial functions.

%% 5.2. Targeting with an E-field model

% This method is currently quite experimental and requires custom software.
% See the open-source GitHub repository: https://github.com/lainem11/E-field_targeting/example_complex_geom.m.
% The example script shows how to optimize the induced E-field pattern for
% stimulation targeting. We will assume you have a saved result from the
% E-field targeting scripts as a file called 'targeting_results.mat'.

% Load targeting results
targeting_results = load('/home/mtms/projects/mTMS_toolkit/targeting_results_TS.mat');
didt = targeting_results.weights;

% Convert coil weights from the E-field targeting results to load voltages
normalized_load_voltages = mtms_tk.didt_to_volts(didt);

% The results are normalized to produce E-field of 1 V/m in strength. Scale
% voltages as needed.
intensity = 30;     % V/m
load_voltages = normalized_load_voltages * intensity;

% Generate pulse structure
clear pulse_sequence
for i = 1:size(load_voltages,1)
    pulse_sequence(i) = mtms_tk.generate_pulse_structure(load_voltages(i,:));
end

% Execute pulse
mtms_tk.run_stimulation_sequence(pulse_sequence);

%%

%%%%% 6. COPY-PASTEABLES %%%%%

%% 6.1. Basic RMT measurement

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
                                                pulse_label = "RMT_measurement", ...
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

