classdef mTMS_toolkit < handle
    %UNTITLED Summary of this class goes here
    %   Detailed explanation goes here

    properties
        api
        mep_configuration
        pulse_sequence
        save_dir
        block_count
        closed_loop_trial_index
        closed_loop_results
        closed_loop_experiment_count
        MEP_time_window
        enable_EMG_filter
        readout_fs
    end

    methods
        function obj = mTMS_toolkit(api,save_dir)
            %UNTITLED Construct an instance of this class
            %   Detailed explanation goes here

            addpath(genpath("/home/mtms/projects/mTMS_toolkit/misc"))
            obj.api = api;
            obj.generate_default_MEP_configuration()
            obj.set_MEP_processing_options([0.015,0.045],5000,1)
            obj.save_dir = save_dir;
            obj.reset_experiment


            if ~isempty(obj.save_dir) && ~exist(obj.save_dir,"dir")
                mkdir(obj.save_dir)
            end
        end

        function reset_experiment(obj)
            obj.block_count = 1;
            obj.closed_loop_trial_index = 1;
            obj.closed_loop_experiment_count = 1;
            obj.closed_loop_results = struct([]);
        end

        function set_MEP_processing_options(obj,MEP_time_window,readout_fs,enable_filter)
            obj.MEP_time_window = MEP_time_window;
            obj.enable_EMG_filter = enable_filter;
            obj.readout_fs = readout_fs;
        end

        function [groupedData,medians] = extract_amplitude_by_label(obj,dataStruct)
            % extractAmplitudeByLabel Groups readout.amplitude by unique label
            %
            % INPUT:
            %   dataStruct - array of structures with fields:
            %                - label: string or char array
            %                - readout: struct with field 'amplitude' (numeric)
            %
            % OUTPUT:
            %   groupedData - structure with fields for each unique label
            %                 Each field contains an array of amplitude values

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

            emg_channel = 0; % indexing starts from 0.
            mep_start_time = -0.2;  % in ms, after the stimulation pulse
            mep_end_time = 0.1;  % in ms
            preactivation_check_enabled = false;
            preactivation_start_time = -0.02;  % in ms, minus sign indicates that the window starts before the stimulation pulse
            preactivation_end_time = -0.01;
            preactivation_voltage_range_limit = 70;  % Maximum allowed voltage range inside the time window, in uV.

            obj.generate_MEP_configuration(emg_channel, mep_start_time, mep_end_time, preactivation_check_enabled, preactivation_start_time, preactivation_end_time, preactivation_voltage_range_limit);
        end

        function waveforms = get_monophasic_reference_waveform(obj)
            % Set reference pulse durations.
            durations =  num2cell([60,30,37.5; ...
                                   60,30,37.5;...
                                   60,30,37.5;...
                                   60,30,37.5;...
                                   60,30,37.5]*1e-6);
            modes = {'r','h','f'};
            for i = 1:size(durations,1)
                waveforms{i} = struct('mode',modes,'duration',durations(i,:));
            end
        end

        function waveform = get_biphasic_reference_waveform(obj)
            % Set reference pulse durations.
            durations =  num2cell([60,40,140,10,67.5; ...
                                   60,40,140,10,67.5;...
                                   60,40,140,10,69.2;...
                                   60,40,140,10,69.2;...
                                   60,40,140,10,73.0]*1e-6);
            modes = {'f','h','r','h','f'};
            waveform = struct('mode',modes,'duration',durations);
        end

        function target_voltages = get_target_voltages(obj,displacement_x,displacement_y,rotation_angle,intensity)
            algorithm = obj.api.get_targeting_algorithm('genetic');
            target = obj.api.create_target(displacement_x, displacement_y, rotation_angle, intensity, algorithm);
            [target_voltages, reverse_polarities] = obj.api.get_target_voltages(target);
            target_voltages(reverse_polarities) = -target_voltages(reverse_polarities);
        end

        function ITI = random_ITI(~,n_trials,ITI_window)
            width = ITI_window(2) - ITI_window(1);
            ITI = rand(n_trials,1);
            ITI = ITI * width + ITI_window(1);
        end

        function pulse_structure = generate_pulse_structure(obj,load_voltages,opt)
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
            N_chn = obj.api.channel_count;
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
            assert(isempty(opt.ISI) || (isnumeric(opt.ISI) && opt.ISI > 0 && opt.ISI < 0.05), sprintf("ISI must be empty, or number between 0 and 0.05."))
            assert(isempty(opt.readout_type) || strcmp(opt.readout_type,'EMG') || strcmp(opt.readout_type,'EEG'), sprintf("readout_type must be empty, 'EMG', or 'EEG'"))
            assert(isempty(opt.repeat_on_failure) || opt.repeat_on_failure == false || opt.repeat_on_failure == true, sprintf("repeat_of_failure must be empty, true, or false."))
            assert(isempty(opt.pulse_label) || ischar(opt.pulse_label), sprintf("pulse_label must be empty or character vector (use '')"))
            assert(isempty(opt.charge_to_stim_time) || (opt.charge_to_stim_time < opt.ITI_window(1) && opt.charge_to_stim_time > 1), sprintf("charge_to_stim_time must be less than the minimum ITI, but larger than 1."))

            % Set default waveforms if not specified
            if isempty(opt.waveforms)
                reverse_polarities = load_voltages < 0;
                % Define single-pulse waveforms
                for j = 1:N_chn
                    opt.waveforms{1,j} = obj.api.get_default_waveform(j-1);
                    if reverse_polarities(j)
                        opt.waveforms{1,j} = obj.api.reverse_polarity(opt.waveforms{1,j});
                    end
                end
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
            % TODO:Add warmup to repeat first pulse,
            % print MEP size, pulse count
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

                    [readout,pulse_ok,p] = obj.stimulate(p);

                    start_time = p.stim_time(1);
                    pulse_times = [pulse_times datetime];

                    if pulse_ok
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
                result(i).pulse = p;
                result(i).readout = readout;
                result(i).time = pulse_times;
                %result(i).timestring = datestr(pulse_times,'dd-mm-yyyy HH:MM:SS:FFF');
                result(i).label = string(p.pulse_label);
            end
            if ~isempty(obj.save_dir)
                obj.save_block_result(result)
            end
            obj.api.send_immediate_full_discharge_to_all_channels();
            obj.api.wait_for_completion();
        end

        function result = run_stimulation_trial(obj,pulse_structure)
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
                [readout,pulse_ok,p] = obj.stimulate(p);

                %start_time = p.stim_time(1);
                pulse_times = [pulse_times datetime];

                if pulse_ok
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
            if ~isempty(obj.save_dir)
                filename = fullfile(obj.save_dir,sprintf("closed_loop_%s.mat",num2str(obj.closed_loop_experiment_count)));
                obj.closed_loop_experiment_count = obj.closed_loop_experiment_count + 1;
                result = obj.closed_loop_results;
                save(filename,"result")
            end
        end

        function save_block_result(obj,result)
            if ~isempty(obj.save_dir)
                filename = fullfile(obj.save_dir,sprintf("block_%s.mat",num2str(obj.block_count)));
                obj.block_count = obj.block_count + 1;
                save(filename,"result")
            end
        end

        function shuffled_pulse_sequence = shuffle_pulse_sequence(obj,pulse_sequence)
            permutation_indices = randperm(numel(pulse_sequence));
            shuffled_pulse_sequence = pulse_sequence(permutation_indices);
        end

        function blocks = split_sequence_to_blocks(obj,pulse_sequence,N_blocks)
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
            % Stimulate with coil 5 with increasing intensity
            max_voltage = 1500;
            min_voltage = 500;
            voltages = linspace(min_voltage,max_voltage*max_intensity,number_of_pulses);
            start_time = obj.api.get_time();
            for i = 1:number_of_pulses
                p = obj.generate_pulse_structure([0,0,0,0,voltages(i)]);
                p.ITI_window = [3,4];
                ITI = obj.random_ITI(1,p.ITI_window);
                p.charge_to_stim_time = 2.5;
                p.stim_time = start_time + ITI;
                p.charge_time = p.stim_time - p.charge_to_stim_time;

                [~,~,p] = obj.stimulate(p);
                start_time = p.stim_time(1);
            end
        end

        function [readout,pulse_ok,p] = stimulate(obj,pulse_structure)
            %
            % Loads capacitors and applies a pulse with default monophasic
            % waveform. Options can be set to run a multi-pulse sequence,
            % specify custom waveforms, measure a readout, and send output
            % trigger from device.
            %
            % Arguments

            %   Optional
            %   opt.load_time (int): Onset of capacitor loading, measured with api.get_time() in seconds.
            %   opt.stim_time (int array): Onset of each stimulus, analogous to opt.load_time. If longer than 1, applies a multi-pulse sequence.
            %   opt.readout_type (char vector): Readout measure. Must be [] (=no readout), 'EMG' or 'EEG'.
            %   opt.trigger_out (int): Output trigger channel of either 1, or 2. Other values skip the trigger send. Trigger coincides with the last pulse, if using a multi-pulse sequence.
            %   opt.waveforms (cell matrix): Waveforms for each channel (columns) and each multi-pulse (rows).
            %   opt.repeat_on_failure (bool): If True, repeat stimulus if any errors occurr during loading, stimulating, or readout (for maximum of 3 times).
            %   opt.success (array):
            % arguments
            %     obj
            %     load_voltages;
            %     opt.load_time = [];
            %     opt.stim_time = [NaN];
            %     opt.readout_type = [];
            %     opt.trigger_out = [];
            %     opt.waveforms = {};
            %     opt.repeat_on_failure = [];
            %     opt.success = [];
            % end
            p = pulse_structure;

            % DEBUG
            % now = obj.api.get_time();
            % fprintf("CHARGE IN %.1f, STIM IN %.1f\n",p.charge_time-now,p.stim_time-now)

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
                if i ~= 6
                    charge_ids(i) = obj.api.send_charge_or_discharge(i-1,abs_load_voltages(i),load_condition);
                end
            end
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
            for i = 1:size(p.waveforms,1)
                for j=1:size(p.waveforms,2)
                    % send_pulse is always timed to cover multi-pulse
                    % scenarios, and maintain constant charge_to_stim time
                    if j ~= 6
                        pulse_ids(i,j) = obj.api.send_pulse(j-1, p.waveforms{i,j}, false, obj.api.execution_conditions.TIMED, stim_time(i));
                    end
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

            obj.api.wait_for_completion(10)

            % Check for any errors
            pulse_ok = 1;
            for id=charge_ids
                feedback = obj.api.get_event_feedback(id);
                if ~(isstruct(feedback) && feedback.value == feedback.NO_ERROR)
                    fprintf('Error in channel charging.\n');
                    pulse_ok = 0;
                end
            end
            pulse_ids = pulse_ids(:);
            for id=1:length(pulse_ids)
                feedback = obj.api.get_event_feedback(pulse_ids(id));
                if ~(isstruct(feedback) && feedback.value == feedback.NO_ERROR)
                    fprintf('Error in pulse generation.\n');
                    pulse_ok = 0;
                end
            end
            if ~isempty(trig_id)
                feedback = obj.api.get_event_feedback(trig_id);
                if ~(isstruct(feedback) && feedback.value == feedback.NO_ERROR)
                    fprintf('Error in output trigger.\n');
                    pulse_ok = 0;
                end
            end
            if isstruct(readout) && isfield(readout,'buffer')
                if isempty(readout.buffer)
                    fprintf('Error in readout.\n');
                    readout_err.gather_mep_error
                    pulse_ok = 0;
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

            if pulse_ok

                % Try to catch problems with coil connections
                load_voltages_post = obj.api.get_current_voltages();
                voltage_drop_ratio = (abs_load_voltages-load_voltages_post)./abs_load_voltages;
                odd_channels = find((voltage_drop_ratio < 0.01) & (abs_load_voltages > 50));
                if ~isempty(odd_channels)
                    channel_str = sprintf('%d,',odd_channels); channel_str(end) = [];
                    warning(sprintf("No voltage drop in channel(s): %s. Was a pulse given?",channel_str))
                end
            end
        end

        function [waveforms,load_voltages_after_pulse, approximated_state_trajectories] = generate_biphasic_PWM_waveforms(obj,target_voltages,load_voltages,reference_durations)
            % Generates waveforms for biphasic PWM pulses.
            % If target_voltages has more than one row, it's assumed to be a
            % multipulse.

            % Turn off function handle warning that clutters the console
            warning('off', 'MATLAB:dispatcher:UnresolvedFunctionHandle');


            solutions_filename = "/home/mtms/mtms/ros2_ws/src/mtms_packages/targeting/waveform_approximator/waveform_approximator/solutions_five_coil_set.mat";
            addpath(genpath("/home/mtms/mtms/ros2_ws/src/mtms_packages/targeting/waveform_approximator"))
            time_resolution = 0.01e-6;

            target_state_trajectories = {};
            approximated_state_trajectories = {};
            %all_sampling_points = {};
            waveforms = {};
            n_pulses = size(target_voltages,1);
            n_channels = size(target_voltages,2);

            % Save voltages before each pulse
            load_voltages_after_pulse = zeros(size(target_voltages));
            % Assumed voltages start from the initial load voltages
            assumed_voltages = load_voltages;

            figure
            tcl = tiledlayout(n_pulses,n_channels);
            for i = 1:n_pulses
                custom_waveforms = {};
                for j = 1:n_channels
                    % Select approximator object and select coil for modelling the
                    % circuits
                    approximator = WaveformApproximator(solutions_filename, time_resolution);
                    approximator.select_coil(j);

                    % Define target waveform for the PWM approximation
                    actual_voltage = assumed_voltages(j);
                    target_voltage = target_voltages(i,j);
                    if target_voltage > 0
                        mode = {'f','h','r','h','f'};
                    else
                        mode = {'r','h','f','h','r'};
                    end
                    durations = num2cell(reference_durations(j,:));
                    target_waveform = struct('mode', mode, 'duration', durations);

                    % Run iterative approximation for the best PWM fit
                    [approximated_waveform, success] = approximator.approximate_iteratively(actual_voltage, abs(target_voltage), target_waveform);

                    custom_waveforms{j} = obj.api.create_waveform(approximated_waveform);

                    % Get model parameters of the electronics circuit during the pulse
                    % to plot the current.
                    target_state_trajectories{i,j} = approximator.generate_state_trajectory_from_waveform(abs(target_voltage), target_waveform);
                    tmp_approximated_state_trajectory = approximator.generate_state_trajectory_from_waveform(actual_voltage, approximated_waveform);
                    approximated_state_trajectories{i,j} = tmp_approximated_state_trajectory;
                    %all_sampling_points{i,j} = sampling_points;

                    % Consider voltage drop in the channel for consecutive pulses
                    % There is a bug that the last value is the initial state
                    assumed_voltages(j) = tmp_approximated_state_trajectory(end-1).V_c;

                    % Plot
                    nexttile
                    approximator.plot_state_trajectories(target_state_trajectories{i,j}, approximated_state_trajectories{i,j})
                end
                load_voltages_after_pulse(i,:) = assumed_voltages;
                title(tcl,'Channel currents. Pulses in rows, channels in columns.')

                waveforms(i,:) = custom_waveforms;
            end

            warning('on', 'MATLAB:dispatcher:UnresolvedFunctionHandle');
        end

        function [waveforms,load_voltages_after_pulse, approximated_state_trajectories] = generate_PWM_waveforms(obj,target_voltages,load_voltages,reference_waveforms)
            % Generates waveforms for biphasic PWM pulses.
            % If target_voltages has more than one row, it's assumed to be a
            % multipulse.

            function waveform = reverse_phases(waveform)
                for k = 1:length(waveform)
                    current_mode = waveform(k).mode;
                    if strcmp(current_mode,'f')
                        waveform(k).mode = 'r';
                    elseif strcmp(current_mode,'r')
                               waveform(k).mode = 'f';
                    end
                end
            end

            % Turn off function handle warning that clutters the console
            warning('off', 'MATLAB:dispatcher:UnresolvedFunctionHandle');


            solutions_filename = "/home/mtms/mtms/ros2_ws/src/mtms_packages/targeting/waveform-approximator/waveform_approximator/solutions_five_coil_set.mat";
            addpath(genpath("/home/mtms/mtms/ros2_ws/src/mtms_packages/targeting/waveform-approximator"))
            time_resolution = 0.01e-6;

            target_state_trajectories = {};
            approximated_state_trajectories = {};
            %all_sampling_points = {};
            waveforms = {};
            n_pulses = size(target_voltages,1);
            n_channels = size(target_voltages,2);

            % Save voltages before each pulse
            load_voltages_after_pulse = zeros(size(target_voltages));
            % Assumed voltages start from the initial load voltages
            assumed_voltages = load_voltages;

            figure
            tcl = tiledlayout(n_pulses,n_channels);
            for i = 1:n_pulses
                custom_waveforms = {};
                for j = 1:n_channels
                    % Select approximator object and select coil for modelling the
                    % circuits
                    approximator = WaveformApproximator(solutions_filename, time_resolution);
                    approximator.select_coil(j);

                    % Define target waveform for the PWM approximation
                    actual_voltage = assumed_voltages(j);
                    target_voltage = target_voltages(i,j);
                    if target_voltage > 0
                        target_waveform = reference_waveforms{i,j};
                    else
                        target_waveform = reverse_phases(reference_waveforms{i,j});
                    end
                    %durations = num2cell(reference_durations(j,:));
                    %target_waveform = struct('mode', mode, 'duration', durations);

                    % Run iterative approximation for the best PWM fit
                    [approximated_waveform, success] = approximator.approximate_iteratively(actual_voltage, abs(target_voltage), target_waveform);

                    custom_waveforms{j} = obj.api.create_waveform(approximated_waveform);

                    % Get model parameters of the electronics circuit during the pulse
                    % to plot the current.
                    target_state_trajectories{i,j} = approximator.generate_state_trajectory_from_waveform(abs(target_voltage), target_waveform);
                    tmp_approximated_state_trajectory = approximator.generate_state_trajectory_from_waveform(actual_voltage, approximated_waveform);
                    approximated_state_trajectories{i,j} = tmp_approximated_state_trajectory;
                    %all_sampling_points{i,j} = sampling_points;

                    % Consider voltage drop in the channel for consecutive pulses
                    % There is a bug that the last value is the initial state
                    assumed_voltages(j) = tmp_approximated_state_trajectory(end-1).V_c;

                    % Plot
                    nexttile
                    approximator.plot_state_trajectories(target_state_trajectories{i,j}, approximated_state_trajectories{i,j})
                end
                load_voltages_after_pulse(i,:) = assumed_voltages;
                title(tcl,'Channel currents. Pulses in rows, channels in columns.')

                waveforms(i,:) = custom_waveforms;
            end

            warning('on', 'MATLAB:dispatcher:UnresolvedFunctionHandle');
        end

        function clean_EMG = filter_EMG(obj,buffer)
            fs = obj.readout_fs;

            % Bandpass
            [b2,a2] = butter(2,[20 500]/(fs/2));
            EMG_bp = filtfilt(b2,a2,buffer);

            % Filter out line-noise
            clean_EMG = ft_preproc_dftfilter(EMG_bp',fs,50,'dftreplace','neighbour');
        end

        function readout = calculate_p2p(obj,readout)
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

    end
end