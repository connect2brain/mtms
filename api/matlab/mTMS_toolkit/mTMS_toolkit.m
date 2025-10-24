classdef mTMS_toolkit < handle
    %   A toolkit for managing multi-locus Transcranial Magnetic Stimulation (mTMS) experiments.
    %   This class provides an interface for configuring and executing TMS experiments,
    %   including pulse generation, waveform configuration, and EMG/EEG data processing.
    %   It interacts with an external API to control hardware and manage experimental parameters.

    properties
        api % External API for interfacing with TMS hardware
        coil_specs % Table for coil specification parameters
        mep_configuration % Configuration for Motor Evoked Potential (MEP) analysis
        pulse_sequence % Sequence of pulses for stimulation
        save_dir % Directory for saving experiment results
        block_count % Counter for experiment blocks
        closed_loop_trial_index % Index for closed-loop trial tracking
        closed_loop_results % Results from closed-loop experiments
        closed_loop_experiment_count % Counter for closed-loop experiments
        MEP_time_window % Time window for MEP analysis (in seconds)
        enable_EMG_filter % Flag to enable/disable EMG filtering
        readout_fs % Sampling frequency for readout data (in Hz)
        cleanupObj % Object to handle path cleanup on object destruction
        channels_in_use % List of channel indices in use
        approximator % Waveform approximator instance
        strain_checker % Instance for pulse strain calculation
    end

    methods
        function obj = mTMS_toolkit(api,coil_spec_file,save_dir)
            % Initializes the toolkit with the provided API and save directory, sets up default
            % MEP configurations, and adds necessary paths for auxiliary functions.
            %
            % :param api: External API object for interfacing with TMS hardware
            % :type api: object
            % :param save_dir: Directory path for saving experiment results
            % :type save_dir: char
            %
            % :return: obj: Instance of the mTMS_toolkit class
            % :rtype: mTMS_toolkit

            % Get path to the misc folder
            classFilePath = mfilename('fullpath');
            classDir = fileparts(classFilePath);
            miscDir = fullfile(classDir, 'misc');

            % Check if the misc folder exists
            if isfolder(miscDir)
                % Add misc folder contents to the MATLAB path
                addpath(genpath(miscDir));
                
                % Set up cleanup to remove the path when the object is destroyed
                obj.cleanupObj = onCleanup(@() rmpath(genpath(miscDir)));
            else
                warning('The misc folder does not exist at: %s', miscDir);
            end

            obj.channels_in_use = [0,1,2,3,4];
            obj.api = api;
            obj.coil_specs = obj.read_coil_specs(coil_spec_file);
            obj.generate_default_MEP_configuration()
            obj.set_MEP_processing_options([0.015,0.045],5000,1)
            obj.save_dir = save_dir;
            obj.reset_experiment

            if ~isempty(obj.save_dir) && ~exist(obj.save_dir,"dir")
                mkdir(obj.save_dir)
            end

            obj.approximator = obj.get_approximator();
            obj.strain_checker = pulse_strain_checker(obj.approximator);
        end

        function set_channels_in_use(obj,channels_in_use)
            % Defines the channels currently in use.
            obj.channels_in_use = channels_in_use;
        end

        function coil_specs = read_coil_specs(~,coil_spec_file)
            % Reads coil specification parameters from file.
            %
            % :param coil_spec_file: Filepath to a tabular (e.g. csv) file.
            % :type coil_spec_file: char

            % :return coil_specs: Coil specification parameters
            % :type coil_specs: Table

            % Check if coil specification file exists
            if ~isfile(coil_spec_file)
                error('mTMS_toolkit:read_coilspecs:FileNotFound', ...
                      'Coil specification file not found at: %s', coil_spec_file);
            end
            
            try
                coil_specs = readtable(coil_spec_file, 'VariableNamingRule', 'preserve');
            catch err
                error('mTMS_toolkit:read_coilspecs:FileReadError', ...
                      'Failed to process coil file: %s', err.message);
            end
        end

        function reset_experiment(obj)
            % Resets the experiment state.
            % Does not require any parameters. Does not return any value.

            obj.block_count = 1;
            obj.closed_loop_trial_index = 1;
            obj.closed_loop_experiment_count = 1;
            obj.closed_loop_results = struct([]);
        end

        function set_MEP_processing_options(obj,MEP_time_window,readout_fs,enable_filter)
            % Sets the time window, sampling frequency, and filtering options for MEP analysis.
            %
            % :param MEP_time_window: Time window for MEP analysis [start, end] in seconds
            % :type MEP_time_window: double array
            % :param readout_fs: Sampling frequency for readout data in Hz
            % :type readout_fs: double
            % :param enable_filter: Flag to enable/disable EMG filtering
            % :type enable_filter: logical
            %
            % :return: No return value

            obj.MEP_time_window = MEP_time_window;
            obj.enable_EMG_filter = enable_filter;
            obj.readout_fs = readout_fs;
        end

        function generate_MEP_configuration(obj,emg_channel, mep_start_time, mep_end_time, preactivation_check_enabled, preactivation_start_time, preactivation_end_time, preactivation_voltage_range_limit)
            % Configure settings for capturing EMG segments.
            %
            % :param emg_channel: The EMG channel number.
            % :type emg_channel: int
            % :param mep_start_time: Start of the time window for MEP analysis, relative to the event of interest (e.g., pulse).
            % :type mep_start_time: float
            % :param mep_end_time: End of the time window for MEP analysis, relative to the event of interest (e.g., pulse).
            % :type mep_end_time: float
            % :param preactivation_check_enabled: Whether to enable the preactivation check.
            % :type preactivation_check_enabled: bool
            % :param preactivation_start_time: Start of the preactivation time window, relative to the event of interest (e.g., pulse).
            % :type mep_start_time: float
            % :param preactivation_end_time: End of the preactivation time window, relative to the event of interest (e.g., pulse).
            % :type mep_end_time: float
            % :param preactivation_voltage_range_limit: Voltage range limit; if the range of voltages within the preactivation time window
            %    exceeds this limit, the preactivation check will fail.
            % :type preactivation_voltage_range_limit: float
            %
            % :return: No return value

            obj.mep_configuration = obj.api.create_mep_configuration(emg_channel, mep_start_time, mep_end_time, preactivation_check_enabled, preactivation_start_time, preactivation_end_time, preactivation_voltage_range_limit);
        end

        function generate_default_MEP_configuration(obj)
            % Configures default settings for capturing EMG segments.
            % Does not require any parameters. Does not return any value.

            emg_channel = 0; % indexing starts from 0.
            mep_start_time = -0.2;  % 200 ms before pulse
            mep_end_time = 0.1;  % 100 ms after pulse
            preactivation_check_enabled = false;
            preactivation_start_time = -0.02;  % 200 ms before pulse
            preactivation_end_time = -0.01;    % 100 ms before pulse
            preactivation_voltage_range_limit = 70;  % Maximum allowed voltage range inside the time window, in uV.

            obj.generate_MEP_configuration(emg_channel, mep_start_time, mep_end_time, preactivation_check_enabled, preactivation_start_time, preactivation_end_time, preactivation_voltage_range_limit);
        end

        function waveforms = get_monophasic_reference_waveforms(obj)
            % Creates a set of predefined monophasic waveform configurations for each channel,
            % specifying pulse modes and durations.
            %
            % :return: waveforms: Cell array of waveform structures with 'mode' and 'duration' fields
            % :rtype: cell

            % Read last phase durations from coil spec table
            if ~ismember('last_phase_duration_monophasic', obj.coil_specs.Properties.VariableNames)
                    error('mTMS_toolkit:get_monophasic_reference_waveforms:InvalidFileFormat', ...
                          'Coil spec file must contain a ''last_phase_duration_monophasic'' column.');
            end
            last_phase_durs = obj.coil_specs.last_phase_duration_monophasic;
            ramp_up_dur = 60e-6;
            hold_dur = 30e-6;

            % Validate format
            n_coils = length(obj.channels_in_use);
            if length(last_phase_durs) ~= n_coils
                error('mTMS_toolkit:get_monophasic_reference_waveforms:InvalidWaveformDurationCount', ...
                      'Expected %d duration values, got %d.', n_coils, length(last_phase_durs));
            end

            % Set reference pulse durations.
            durations =  num2cell([repmat(ramp_up_dur,n_coils,1), ...
                                   repmat(hold_dur,n_coils,1), ...
                                   last_phase_durs]);

            % Define the default waveform
            modes = {'r','h','f'};
            for i = 1:size(durations,1)
                waveforms{1,i} = struct('mode',modes,'duration',durations(i,:));
            end
        end

        function waveforms = get_biphasic_reference_waveforms(obj)
            % Creates a set of predefined biphasic waveform configurations for each channel,
            % specifying pulse modes and durations.
            %
            % :return: waveforms: Cell array of waveform structures with 'mode' and 'duration' fields
            % :rtype: cell

            % Read last phase durations from coil spec table
            if ~ismember('last_phase_duration_biphasic', obj.coil_specs.Properties.VariableNames)
                    error('mTMS_toolkit:get_biphasic_reference_waveforms:InvalidFileFormat', ...
                          'Coil spec file must contain a ''last_phase_duration_biphasic'' column.');
            end
            last_phase_durs = obj.coil_specs.last_phase_duration_biphasic;
            ramp_up1_dur = 60e-6;
            hold1_dur = 40e-6;
            ramp_down_dur = 140e-6;
            hold2_dur = 10e-6;

            % Validate format
            n_coils = length(obj.channels_in_use);
            if length(last_phase_durs) ~= n_coils
                error('mTMS_toolkit:get_biphasic_reference_waveforms:InvalidWaveformDurationCount', ...
                      'Expected %d duration values, got %d.', n_coils, length(last_phase_durs));
            end

            % Set reference pulse durations.
            durations =  num2cell([repmat(ramp_up1_dur,n_coils,1), ...
                                   repmat(hold1_dur,n_coils,1), ...
                                   repmat(ramp_down_dur,n_coils,1), ...
                                   repmat(hold2_dur,n_coils,1), ...
                                   last_phase_durs]);

            % Define the default waveform
            modes = {'f','h','r','h','f'};
            for i = 1:size(durations,1)
                waveforms{1,i} = struct('mode',modes,'duration',durations(i,:));
            end
        end

        function reversed_waveform = reverse_waveform(~,waveform)
            % Reverses the rise and fall phases of a waveform.
            %
            % :param waveform: Waveform modes and durations
            % :type waveform: struct
            %
            % :return: reversed_waveform: Waveform with reversed modes.
            % :rtype: struct array
            reversed_waveform = waveform;
            for i = 1:length(waveform)
                switch waveform(i).mode
                    case 'r'
                        reversed_waveform(i).mode = 'f';
                    case 'f'
                        reversed_waveform(i).mode = 'r';
                end
            end
        end

        function api_waveforms = create_waveform_from_struct(obj,waveforms)
            % Transforms wavefrom modes and durations to a waveform object.
            %
            % :param waveforms: Waveform modes and durations
            % :type waveforms: cell array
            %
            % :return: api_waveforms: Waveform objects.
            % :rtype: cell array

            for i = 1:size(waveforms,1)
                for j = 1:size(waveforms,2)
                    api_waveforms{i,j} = obj.api.create_waveform(waveforms{i,j});
                end
            end
        end

        function target_voltages = get_target_voltages(obj,displacement_x,displacement_y,rotation_angle,intensity)
            % Uses a precomputed table to find target voltages based on spatial displacement,
            % rotation, and intensity, adjusting for polarity as needed.
            %
            % :param displacement_x: X-axis displacement of the target (in mm)
            % :type displacement_x: double
            % :param displacement_y: Y-axis displacement of the target (in mm)
            % :type displacement_y: double
            % :param rotation_angle: Rotation angle of the target (in degrees)
            % :type rotation_angle: double
            % :param intensity: Stimulation intensity
            % :type intensity: double
            %
            % :return: target_voltages: Array of computed target voltages
            % :rtype: double array

            algorithm = obj.api.get_targeting_algorithm('genetic');
            target = obj.api.create_target(displacement_x, displacement_y, rotation_angle, intensity, algorithm);
            [target_voltages, reverse_polarities] = obj.api.get_target_voltages(target);
            target_voltages(reverse_polarities) = -target_voltages(reverse_polarities);
            target_voltages = target_voltages';
        end

        function ITI = random_ITI(~,n_trials,ITI_window)
            % Produces a vector of random ITI values within the specified window for the given
            % number of trials.
            %
            % :param n_trials: Number of trials
            % :type n_trials: int
            % :param ITI_window: Interval range [min, max] for ITI (in seconds)
            % :type ITI_window: double array
            %
            % :return: ITI: Array of random inter-trial intervals
            % :rtype: double array

            width = ITI_window(2) - ITI_window(1);
            ITI = rand(n_trials,1);
            ITI = ITI * width + ITI_window(1);
        end

        function pulse_structure = repeat_pulse_structure(~, pulse_structure,N)
            % Creates N copies of the given pulse_structure.
            %
            % :param pulse_strucure: Structure containing pulse configurations
            % :type pulse_structure: struct
            % :param N: Number of copies.
            % :type N: int
            %
            % :return: pulse_structure: Structure containing copies of the
            % original pulse configurations.

            pulse_structure = repmat(pulse_structure,1,N);
        end

        function pulse_structure = generate_pulse_structure(obj,load_voltages,opt)
            % Constructs a pulse structure with specified voltages and optional parameters for
            % waveforms, timing, and readout settings, with input validation.
            %
            % :param load_voltages: Voltages to load for each channel
            % :type load_voltages: double array
            % :param opt: Optional structure with fields:
            %   - charge_to_stim_time: Time from charge to stimulation (in seconds)
            %   - waveforms: Cell array of waveform configurations
            %   - ITI_window: Inter-trial interval range [min, max] (in seconds)
            %   - ISI: Inter-stimulus intervals for multi-pulse sequences (in seconds)
            %   - trigger_out: Output trigger channel (1 or 2)
            %   - readout_type: Type of readout ('EMG' or 'EEG')
            %   - repeat_on_failure: Whether to repeat on failure (logical)
            %   - pulse_label: Label for the pulse (char)
            % :type opt: struct
            %
            % :return: pulse_structure: Structure containing pulse configuration
            % :rtype: struct

            arguments
                obj
                load_voltages;
                opt.charge_to_stim_time = [];
                opt.waveforms = {};
                opt.ITI_window = [];
                opt.ISI = [];
                opt.trigger_out = [];
                opt.readout_type = [];
                opt.repeat_on_failure = [];
                opt.pulse_label = '';
            end
            N_chn = length(obj.channels_in_use);
            if isempty(opt.waveforms)
                N_pulses = 1;
            else
                N_pulses = size(opt.waveforms,1);
            end

            assert(length(load_voltages) == N_chn, sprintf("Expected %i load voltage channels, got %i.",N_chn,length(load_voltages)))
            if ~(isempty(opt.waveforms))
                assert(size(opt.waveforms,2) == N_chn, sprintf("Expected %i waveforms, got %i.",N_chn,size(opt.waveforms,2)))
                assert((N_pulses - 1) == length(opt.ISI), sprintf("Expected %i inter-stimulus intervals, got %i.",N_pulses,length(opt.ISI)))
            end
            assert(isempty(opt.trigger_out) || (opt.trigger_out == 1) || (opt.trigger_out == 2), sprintf("trigger_out must be empty, 1, or 2."))
            assert(isempty(opt.ITI_window) || (opt.ITI_window(1) >= 2 && opt.ITI_window(2) < 20), sprintf("ITI_window must be empty, or inside window of [2,20]."))
            assert(isempty(opt.ISI) || (isnumeric(opt.ISI) && all(opt.ISI > 0) && all(opt.ISI < 0.2)), sprintf("ISI must be empty, or array of numbers between 0 and 0.2."))
            assert(isempty(opt.readout_type) || strcmp(opt.readout_type,'EMG') || strcmp(opt.readout_type,'EEG'), sprintf("readout_type must be empty, 'EMG', or 'EEG'"))
            assert(isempty(opt.repeat_on_failure) || opt.repeat_on_failure == false || opt.repeat_on_failure == true, sprintf("repeat_of_failure must be empty, true, or false."))
            assert(isempty(opt.pulse_label) || ischar(opt.pulse_label), sprintf("pulse_label must be empty or character vector (use '')"))
            assert(isempty(opt.charge_to_stim_time) || (opt.charge_to_stim_time < opt.ITI_window(1) && opt.charge_to_stim_time > 1), sprintf("charge_to_stim_time must be less than the minimum ITI, but larger than 1."))

            % Set default waveforms (single pulse) if not specified
            if isempty(opt.waveforms)
                reverse_polarities = load_voltages < 0;
                opt.waveforms = obj.get_monophasic_reference_waveforms();
                % Define single-pulse waveforms
                for j = 1:N_chn
                    if reverse_polarities(j)
                        opt.waveforms{j} = obj.reverse_waveform(opt.waveforms{j});
                    end
                end
            end

            % Check pulse strain
            [strain_ok, stimulation_intensity_multiplier] = obj.strain_checker.check_pulse_strain(load_voltages,opt.waveforms);
            if ~strain_ok
                error("Pulse strain too high. Maximum stimulation strength is %.0f%s of the suggested.\n",stimulation_intensity_multiplier*100,'%')
            end

            % Set repeat_of_failure to true if not specified
            if isempty(opt.repeat_on_failure)
                opt.repeat_on_failure = true;
            end

            % Set default pulse labels if not specified
            if isempty(opt.pulse_label)
                opt.pulse_label = strjoin(string(round(load_voltages)), '_');
            end

            % Construct pulse structure
            pulse_structure.load_voltages = load_voltages;
            pulse_structure.charge_to_stim_time = opt.charge_to_stim_time;
            pulse_structure.waveforms = opt.waveforms;
            pulse_structure.ITI_window = opt.ITI_window;
            pulse_structure.ISI = opt.ISI;
            pulse_structure.trigger_out = opt.trigger_out;
            pulse_structure.readout_type = opt.readout_type;
            pulse_structure.repeat_on_failure = opt.repeat_on_failure;
            pulse_structure.pulse_label = opt.pulse_label;
        end

        function result = run_stimulation_sequence(obj,pulse_sequence)
            % Runs a sequence of pulses, handling timing, readouts, and retries on failure.
            % Saves results to the specified directory and performs a full discharge afterward.
            %
            % :param pulse_sequence: Array of pulse structures to execute
            % :type pulse_sequence: struct array
            %
            % :return: result: Array of structures containing pulse data, readouts, and timestamps
            % :rtype: struct array

            result = [];
            max_attempts = 5;

            % Restart session (helps with readout connections)
            obj.api.stop_session();
            obj.api.start_session();

            start_time = obj.api.get_time();
            for i = 1:length(pulse_sequence)
                p = pulse_sequence(i);
                p.success = [];

                pulse_complete = false;
                pulse_times = [];

                fprintf("\nPulse %i.\n\n",i)

                while ~pulse_complete
                    % Set inter-trial interval with randomized interval
                    if isempty(p.ITI_window)
                        p.ITI_window = [4,6];
                    end
                    ITI = obj.random_ITI(1,p.ITI_window);

                    % The code maintains a constant charge-to-stimulation delay,
                    % and applies pulse after ISI time has passed.
                    if isempty(p.charge_to_stim_time)
                        p.charge_to_stim_time = mean(p.ITI_window)/2;
                        if p.charge_to_stim_time > p.ITI_window(1)
                            p.charge_to_stim_time = p.ITI_window(1)-0.2;
                        end
                    end
                    p.stim_time = start_time + ITI;
                    p.charge_time = p.stim_time - p.charge_to_stim_time;

                    [readout,pulse_diagnostic] = obj.stimulate(p);

                    start_time = p.stim_time(1);
                    pulse_times = [pulse_times datetime];

                    if pulse_diagnostic.ok_flag
                        p.success = [p.success,1];
                        pulse_complete = true;
                    else
                        p.success = [p.success,0];
                        if p.repeat_on_failure && length(p.success) < max_attempts
                            fprintf('Re-trying, attempt %i...\n',length(p.success))
                            % Restarting the session usually helps with readout issues
                            if pulse_diagnostic.only_readout_failed
                                obj.api.stop_session();
                                obj.api.start_session();
                            end
                        else
                            fprintf('Continuing to next trial...\n')
                            pulse_complete = true;
                        end
                    end
                end
                result(i).pulse = p;
                result(i).readout = readout;
                result(i).time = pulse_times;
                result(i).label = string(p.pulse_label);
            end
            if ~isempty(obj.save_dir)
                obj.save_block_result(result)
            end
            obj.full_discharge()
            obj.api.wait_for_completion(10);
        end

        function full_discharge(obj)
            % Discharges all used channels completely.
            for i = obj.channels_in_use
                obj.api.send_charge_or_discharge(i,0,obj.api.execution_conditions.IMMEDIATE);
            end
        end

        function result = run_stimulation_trial(obj,pulse_structure)
            % Runs a single pulse trial, handling readouts and retries on failure, and stores
            % results in the closed-loop results structure.
            %
            % :param pulse_structure: Structure defining the pulse configuration
            % :type pulse_structure: struct
            %
            % :return: result: Structure containing pulse data, readout, and timestamp
            % :rtype: struct

            p = pulse_structure;
            assert(length(p) == 1, "pulse_structure is expected to have only 1 element.")
            assert(~((isfield(p,'charge_time') || isfield(p,'stim_time')) && p.repeat_on_failure),"Specific charge or stimulation times are not compatible with repeat_on_failure set to true.")

            result = [];
            p.success = [];
            max_attempts = 5;

            pulse_complete = false;
            pulse_times = [];

            fprintf("\nPulse %i.\n\n",obj.closed_loop_trial_index)

            while ~pulse_complete
                [readout,pulse_diagnostic] = obj.stimulate(p);

                pulse_times = [pulse_times datetime];

                if pulse_diagnostic.ok_flag
                    p.success = [p.success,1];
                    pulse_complete = true;
                else
                    p.success = [p.success,0];
                    if p.repeat_on_failure && length(p.success) < max_attempts
                        fprintf('Re-trying, attempt %i...\n',length(p.success))
                    else
                        fprintf('Continuing to next trial...\n')
                        pulse_complete = true;
                    end
                end
            end

            result.pulse = p;
            result.readout = readout;
            result.time = pulse_times;
            result.label = string(p.pulse_label);

            if isempty(obj.closed_loop_results)
                % First entry, accept any struct
                obj.closed_loop_results = result;
            else
                obj.closed_loop_results(obj.closed_loop_trial_index) = result;
            end
            obj.closed_loop_trial_index = obj.closed_loop_trial_index + 1;
        end

        function save_closed_loop_results(obj)
            % Saves the closed-loop results to a .mat file in the specified save directory,
            % incrementing the experiment count.
            %
            % :return: No return value

            if ~isempty(obj.save_dir)
                filename = fullfile(obj.save_dir,sprintf("closed_loop_%s.mat",num2str(obj.closed_loop_experiment_count)));
                obj.closed_loop_experiment_count = obj.closed_loop_experiment_count + 1;
                result = obj.closed_loop_results;
                save(filename,"result")
            end
        end

        function save_block_result(obj,result)
            % Saves the results of a stimulation block to a .mat file in the specified save
            % directory, incrementing the block count.
            %
            % :param result: Structure array containing block results
            % :type result: struct array
            %
            % :return: No return value

            if ~isempty(obj.save_dir)
                filename = fullfile(obj.save_dir,sprintf("block_%s.mat",num2str(obj.block_count)));
                obj.block_count = obj.block_count + 1;
                save(filename,"result")
            end
        end

        function shuffled_pulse_sequence = shuffle_pulse_sequence(obj,pulse_sequence)
            % Reorders the pulses in the sequence randomly using a permutation.
            %
            % :param pulse_sequence: Array of pulse structures to shuffle
            % :type pulse_sequence: struct array
            %
            % :return: shuffled_pulse_sequence: Shuffled array of pulse structures
            % :rtype: struct array

            permutation_indices = randperm(numel(pulse_sequence));
            shuffled_pulse_sequence = pulse_sequence(permutation_indices);
        end

        function blocks = split_sequence_to_blocks(obj,pulse_sequence,N_blocks)
            % Divides the pulse sequence into approximately equal blocks.
            %
            % :param pulse_sequence: Array of pulse structures to split
            % :type pulse_sequence: struct array
            % :param N_blocks: Number of blocks to create
            % :type N_blocks: int
            %
            % :return: blocks: Cell array of pulse sequence blocks
            % :rtype: cell
            blocks = {};
            block_size = ceil(length(pulse_sequence)/N_blocks);
            start = 1;
            for i = 1:N_blocks
                if i ~= N_blocks
                    blocks{i} = pulse_sequence(start:(start+block_size-1));
                    start = start + block_size;
                else
                    blocks{i} = pulse_sequence(start:end);
                end
            end

        end

        function warmup(obj,number_of_pulses,max_intensity)
            % Executes a series of pulses on coil 5 with linearly increasing voltages to prepare
            % the system or subject for stimulation.
            %
            % :param number_of_pulses: Number of warmup pulses
            % :type number_of_pulses: int
            % :param max_intensity: Maximum intensity as a fraction of maximum voltage
            % :type max_intensity: double
            %
            % :return: No return value

            max_voltage = 1500;
            min_voltage = 300;
            voltages = linspace(min_voltage,max_voltage*max_intensity,number_of_pulses);
            start_time = obj.api.get_time();
            for i = 1:number_of_pulses
                p = obj.generate_pulse_structure([0,0,0,0,voltages(i)]);
                p.ITI_window = [3,4];
                ITI = obj.random_ITI(1,p.ITI_window);
                p.charge_to_stim_time = 2.5;
                p.stim_time = start_time + ITI;
                p.charge_time = p.stim_time - p.charge_to_stim_time;

                obj.stimulate(p);
                start_time = p.stim_time(1);
            end
        end

        function [readout,pulse_diagnostic] = stimulate(obj,pulse_structure)
            % Loads capacitors and applies a pulse with the specified configuration, handling
            % waveforms, timing, and readouts (EMG/EEG). Includes error checking and retry logic.
            %
            % :param pulse_structure: Structure defining the pulse configuration
            % :type pulse_structure: struct
            %
            % :return: readout: Readout data (EMG/EEG) or empty if none
            % :rtype: struct
            % :return: pulse_diagnostic: Feedback for successful stimulation
            % :rtype: struct

            p = pulse_structure;

            % Handle electric current polarity
            abs_load_voltages = abs(p.load_voltages);

            if isfield(p,'charge_time') && isfield(p,'stim_time')
                % Wait until charge time
                time_until_charge = p.charge_time - obj.api.get_time();
                if time_until_charge > 0
                    obj.api.wait(time_until_charge)
                else
                    % Offset stim time to maintain constant charge-to-stim time.
                    p.stim_time = p.stim_time + abs(time_until_charge);
                end
            end

            % Load the capacitors.
            charge_ids = [];
            load_condition = obj.api.execution_conditions.IMMEDIATE;
            for i = 1:size(p.waveforms,2)
                charge_ids(i) = obj.api.send_charge_or_discharge(obj.channels_in_use(i),abs_load_voltages(i),load_condition);
            end

            % Prepare waveforms
            waveforms = obj.create_waveform_from_struct(p.waveforms);
            obj.api.wait_for_completion(10);

            if ~isfield(p,'stim_time')
                stim_time = obj.api.get_time()+0.7; % Need time for preparing readout buffer
            else
                stim_time = p.stim_time;
            end

            % Add multi-pulse timings to stim_time
            if ~isempty(p.ISI)
                for i = 1:length(p.ISI)
                    stim_time(end+1) = stim_time(end) + p.ISI(i);
                end
            end

            % Stimulate
            pulse_ids = [];
            for i = 1:size(waveforms,1)
                for j=1:size(waveforms,2)
                    % send_pulse is always timed to cover multi-pulse
                    % scenarios, and maintain constant charge_to_stim time
                    pulse_ids(i,j) = obj.api.send_pulse(obj.channels_in_use(j), waveforms{i,j}, false, obj.api.execution_conditions.TIMED, stim_time(i));
                end
            end

            % Set output trigger (when last pulse is given)
            trig_id = [];
            if ismember(p.trigger_out,[1,2])
                trig_id = obj.api.send_trigger_out(p.trigger_out, 1000, obj.api.execution_conditions.TIMED,stim_time(end));
            end

            % Measure readout (when last pulse is given)
            if ~isempty(p.readout_type)
                if strcmp(p.readout_type,'EMG')
                    [readout,readout_err] = obj.api.analyze_mep(stim_time(end), obj.mep_configuration);
                elseif strcmp(p.readout_type,'EEG')
                    % TODO
                    readout = [];
                end
            else
                readout = [];
            end

            obj.api.wait_for_completion(5)

            % Check for any errors
            pulse_diagnostic.ok_flag = 1;

            pulse_diagnostic.charge_ids = charge_ids;
            for id=charge_ids
                feedback = obj.api.get_event_feedback(id);
                if ~(isstruct(feedback) && feedback.value == feedback.NO_ERROR)
                    fprintf('Error in channel charging.\n');
                    pulse_diagnostic.ok_flag = 0;
                end
            end

            pulse_diagnostic.pulse_ids = pulse_ids;
            pulse_ids = pulse_ids(:);
            for id=1:length(pulse_ids)
                feedback = obj.api.get_event_feedback(pulse_ids(id));
                if ~(isstruct(feedback) && feedback.value == feedback.NO_ERROR)
                    fprintf('Error in pulse generation.\n');
                    pulse_diagnostic.ok_flag = 0;
                end
            end

            pulse_diagnostic.trig_id = trig_id;
            if ~isempty(trig_id)
                feedback = obj.api.get_event_feedback(trig_id);
                if ~(isstruct(feedback) && feedback.value == feedback.NO_ERROR)
                    fprintf('Error in output trigger.\n');
                    pulse_diagnostic.ok_flag = 0;
                end
            end
            
            pulse_diagnostic.only_readout_failed = 0;
            if isstruct(readout) && isfield(readout,'buffer')
                if isempty(readout.buffer)
                    fprintf('Error in readout.\n');
                    if pulse_diagnostic.ok_flag
                        pulse_diagnostic.only_readout_failed = 1;
                    end
                    pulse_diagnostic.ok_flag = 0;
                else
                    if strcmp(p.readout_type,'EMG')
                        % Process buffer
                        readout.buffer_filt = obj.filter_EMG(readout.buffer);
                        readout = obj.calculate_p2p(readout);
                        plot_mep_data(readout, obj.mep_configuration, 5)
                        %fprintf(2,"EMG p2p: %.2d\n",readout.amplitude);
                    end
                end
            end

            if pulse_diagnostic.ok_flag
                % Try to catch problems with coil connections
                load_voltages_post = obj.api.get_current_voltages();
                load_voltages_post = load_voltages_post(obj.channels_in_use+1);
                voltage_drop_ratio = (abs_load_voltages(:)-load_voltages_post(:))./abs_load_voltages(:);
                odd_channels = find((voltage_drop_ratio < 0.01) & (abs_load_voltages(:) > 50));
                if ~isempty(odd_channels)
                    channel_str = sprintf('%d,',odd_channels); channel_str(end) = [];
                    warning(sprintf("No voltage drop in channel(s): %s. Was a pulse given?",channel_str))
                end
            end
        end

        function [waveforms,load_voltages_after_pulse, approximated_state_trajectories, target_state_trajectories] = generate_PWM_waveforms(obj,target_voltages,load_voltages,reference_waveforms)
            % Creates pulse-width modulated (PWM) waveforms for each channel and pulse,
            % approximating target voltages and tracking voltage changes.
            %
            % :param target_voltages: Target voltages for each pulse and channel
            % :type target_voltages: double array
            % :param load_voltages: Initial load voltages for each channel
            % :type load_voltages: double array
            % :param reference_waveforms: Reference waveform configurations
            % :type reference_waveforms: cell array
            %
            % :return: waveforms: Cell array of generated PWM waveforms
            % :rtype: cell
            % :return: load_voltages_after_pulse: Voltages after each pulse
            % :rtype: double array
            % :return: approximated_state_trajectories: Trajectories of circuit states
            % :rtype: cell

            plot_flag = 0; % Change to 1 for debugging.

            target_state_trajectories = {};
            approximated_state_trajectories = {};
            waveforms = {};
            n_pulses = size(target_voltages,1);
            n_channels = size(target_voltages,2);

            % Save voltages before each pulse
            load_voltages_after_pulse = zeros(size(target_voltages));
            % Assumed voltages start from the initial load voltages
            assumed_voltages = load_voltages;
            
            if plot_flag; figure; end

            tcl = tiledlayout(n_pulses,n_channels);
            for i = 1:n_pulses
                for j = 1:n_channels
                    % Select approximator object and select coil for modelling the circuits
                    obj.approximator.select_coil(j);

                    % Define target waveform for the PWM approximation
                    actual_voltage = assumed_voltages(j);
                    target_voltage = target_voltages(i,j);
                    if target_voltage >= 0
                        target_waveform = reference_waveforms{i,j};
                    else
                        target_waveform = obj.reverse_waveform(reference_waveforms{i,j});
                    end

                    % Run iterative approximation for the best PWM fit
                    [approximated_waveform, success] = obj.approximator.approximate_iteratively(actual_voltage, abs(target_voltage), target_waveform);
                    
                    waveforms{i,j} = approximated_waveform;

                    % Get model parameters of the electronics circuit during the pulse
                    % to plot the current.
                    target_state_trajectories{i,j} = obj.approximator.generate_state_trajectory_from_waveform(abs(target_voltage), target_waveform);
                    tmp_approximated_state_trajectory = obj.approximator.generate_state_trajectory_from_waveform(actual_voltage, approximated_waveform);
                    approximated_state_trajectories{i,j} = tmp_approximated_state_trajectory;

                    % Consider voltage drop in the channel for consecutive pulses
                    % There is a bug that the last value is the initial state
                    assumed_voltages(j) = tmp_approximated_state_trajectory(end-1).V_c;

                    % Plot
                    if plot_flag
                        nexttile
                        obj.approximator.plot_state_trajectories(target_state_trajectories{i,j}, approximated_state_trajectories{i,j})
                    end
                end
                load_voltages_after_pulse(i,:) = assumed_voltages;
                title(tcl,'Channel currents. Pulses in rows, channels in columns.')

            end
        end

        function approximator = get_approximator(obj)
            warning('off', 'MATLAB:dispatcher:UnresolvedFunctionHandle');
            solutions_filename = "/home/mtms/mtms/ros2_ws/src/mtms_packages/targeting/waveform_approximator/waveform_approximator/solutions_five_coil_set.mat";
            
            addpath(genpath("/home/mtms/mtms/ros2_ws/src/mtms_packages/targeting/waveform_approximator"))
            time_resolution = 0.01e-6;

            approximator = WaveformApproximator(solutions_filename, time_resolution);
            warning('on', 'MATLAB:dispatcher:UnresolvedFunctionHandle');
        end

        function clean_EMG = filter_EMG(obj,buffer)
            % Applies bandpass and line-noise filters to the EMG buffer to produce clean data.
            %
            % :param buffer: Raw EMG data buffer
            % :type buffer: double array
            %
            % :return: clean_EMG: Filtered EMG data
            % :rtype: double array

            fs = obj.readout_fs;

            % Bandpass
            [b2,a2] = butter(2,[20 500]/(fs/2));
            EMG_bp = filtfilt(b2,a2,buffer);

            % Filter out line-noise
            clean_EMG = spectrum_interpolation_filter(EMG_bp',fs,50);
        end

        function readout = calculate_p2p(obj,readout)
            % Computes the peak-to-peak amplitude within the MEP time window and updates
            % the readout structure with amplitude and peak indices.
            %
            % :param readout: Structure containing EMG data buffer
            % :type readout: struct
            %
            % :return: readout: Updated structure with amplitude and peak indices
            % :rtype: struct

            mep_window = obj.MEP_time_window*1000;
            buffer_window = [obj.mep_configuration.time_window.start, obj.mep_configuration.time_window.end];

            time = (buffer_window(1) + (0:length(readout.buffer)-1) / obj.readout_fs) * 1000;  % Time in ms
            mep_mask = time > mep_window(1) & time < mep_window(2);
            mep = readout.buffer_filt(mep_mask);
            [max_mep,max_mep_ind] = max(mep);
            [min_mep,min_mep_ind] = min(mep);
            readout.amplitude = abs(max_mep - min_mep);
            mep_start_ind = find(mep_mask,1)-1;
            readout.max_ind = mep_start_ind+max_mep_ind;
            readout.min_ind = mep_start_ind+min_mep_ind;
        end

        function [groupedData,medians] = extract_amplitude_by_label(obj,dataStruct)
            % Organizes amplitude data from a structure array by unique labels and computes
            % median amplitudes for each label.
            %
            % :param dataStruct: Array of structures with 'label' and 'readout.amplitude' fields
            % :type dataStruct: struct array
            %
            % :return: groupedData: Structure with fields for each unique label containing amplitude arrays
            % :rtype: struct
            % :return: medians: Structure with median amplitudes for each label
            % :rtype: struct

            % Extract labels
            labels = arrayfun(@(s) string(s.label), dataStruct);

            % Get unique labels
            uniqueLabels = unique(labels);

            % Initialize outputs
            groupedData = struct();
            medians = struct();

            % Loop through each unique label
            for i = 1:numel(uniqueLabels)
                lbl = uniqueLabels(i);
                idx = labels == lbl;
                amplitudes = arrayfun(@(s) s.readout.amplitude, dataStruct(idx));

                fieldName = matlab.lang.makeValidName(lbl);
                groupedData.(fieldName) = amplitudes;
                medians.(fieldName) = median(amplitudes);
            end
        end

        function load_voltages = didt_to_volts(obj, didt, coilset)
            % Converts the rate of change of current (di/dt) to corresponding capacitor load
            % voltages using coil inductance values from a specified file. The input di/dt
            % array must match the number of channels defined by the API.
            %
            % :param didt: Rate of change of current (di/dt) values for each channel (in A/s)
            % :type didt: double array
            %
            % :return: load_voltages: Corresponding capacitor load voltages (in V)
            % :rtype: double array
            %
            % :throws: Error if inductances are missing, or di/dt size does not match channel count
            
            % Ensure di/dt size matches the number of coils
            n_coils = length(obj.channels_in_use);
            if length(didt) ~= n_coils
                error('mTMS_toolkit:didt_to_volts:InvalidInput', ...
                      'Expected di/dt array of length %d, got %d.', n_coils, length(didt));
            end

            % Read coil specifications
            if ~ismember('inductances', obj.coil_specs.Properties.VariableNames)
                error('mTMS_toolkit:didt_to_volts:InvalidFileFormat', ...
                      'Coil file must contain an ''inductances'' column.');
            end
            inductances = obj.coil_specs.inductances';
            
            % Validate inductances
            if length(inductances) ~= n_coils
                error('mTMS_toolkit:didt_to_volts:InvalidInductanceCount', ...
                      'Expected %d inductance values, got %d.', n_coils, length(inductances));
            end

            if nargin >= 2
                field_name = ['polarities_vs_', coilset];
                if ~ismember(field_name, obj.coil_specs.Properties.VariableNames)
                error('mTMS_toolkit:didt_to_volts:InvalidFileFormat', ...
                      'Coil file must contain a %s column.',field_name);
                end

                % Specify mismatch between assumed E-field model and reality
                polarities = obj.coil_specs.(field_name)';

                if length(polarities) ~= n_coils
                    error('mTMS_toolkit:didt_to_volts:InvalidInductanceCount', ...
                          'Expected %d inductance values, got %d.', n_coils, length(polarities));
                end
            else
                polarities = ones(1,n_coils);   % No change in polarities
            end
            
            % Calculate load voltages using V = L * di/dt
            load_voltages = inductances .* polarities.* didt;
        end

    end
end
