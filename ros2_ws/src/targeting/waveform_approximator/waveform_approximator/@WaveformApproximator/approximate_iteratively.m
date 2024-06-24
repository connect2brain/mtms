function [aggregated_approximated_waveform, sampling_points, success] = approximate_iteratively(obj, actual_voltage, target_voltage, target_waveform)
 
    % Generate initial conditions.
    initial_state = obj.generate_initial_state(actual_voltage);

    % Check that the target voltage does not exceed the actual voltage.
    if target_voltage > actual_voltage
        error('The target voltage exceeds the actual voltage.');
    end

    relative_error_threshold = 0.02;

    % Generate the state trajectory for the target waveform.
    state_trajectory = obj.generate_state_trajectory_from_waveform(target_voltage, target_waveform);

    % Define the separate algorithm options for modes pre and post hold
    % phases.
    algorithms_pre_hold = {};
    algorithms_post_hold = {};

    % List algorithms with different intermediate points, which ensure the
    % hold phase is always equal length with the target mode.
    for i = 3:-2:1
        algorithm = struct( ...
            'algorithm', @obj.algorithm_alternating_hold_start_with_non_hold, ...
            'algorithm_name', 'alternating hold start with non-hold', ...
            'intermediate_points', i, ...
            'check_duration', true, ...
            'check_relative_error', true);

        %algorithms{end+1} = algorithm;
        algorithms_post_hold{end+1} = algorithm;
    end
    for i = 2:-2:0
        algorithm = struct( ...
            'algorithm', @obj.algorithm_alternating_hold_start_with_hold, ...
            'algorithm_name', 'alternating hold start with hold', ...
            'intermediate_points', i, ...
            'check_duration', true, ...
            'check_relative_error', true);

        %algorithms{end+1} = algorithm;
        algorithms_pre_hold{end+1} = algorithm;
    end

    % Add the micropulse and constant hold algorithms with 0 intermediate points
    micropulse_algorithm = struct( ...
        'algorithm', @obj.algorithm_micropulse, ...
        'algorithm_name', 'micropulse', ...
        'intermediate_points', 0, ...
        'check_duration', true, ...
        'check_relative_error', true);

    % The constant hold is only used for very low target voltages, where the approximation
    % might contain large relative errors. For that reason, the relative errors are not checked.
    constant_hold_algorithm = struct( ...
        'algorithm', @obj.algorithm_constant_hold, ...
        'algorithm_name', 'constant hold', ...
        'intermediate_points', 0, ...
        'check_duration', true, ...
        'check_relative_error', false);

    algorithms_pre_hold{end+1} = micropulse_algorithm;
    algorithms_pre_hold{end+1} = constant_hold_algorithm;
    algorithms_post_hold{end+1} = micropulse_algorithm;
    algorithms_post_hold{end+1} = constant_hold_algorithm;

    % Loop through the modes and for each mode loop trough the algorithms
    modes = [target_waveform.mode];
    durations = [target_waveform.duration];
    state_trajectory_inds = floor(cumsum(durations/obj.time_resolution))+1;
    state_trajectory_start_ind = 1;
    aggregated_approximated_waveform = struct('mode', {}, 'duration', {});
    success = false;
    pre_hold = 1;

    figure(1)
    for m = 1:length(modes)
        wavelet = struct('mode', modes(m), 'duration', durations(m));
        wavelet_state_trajectory = state_trajectory(state_trajectory_start_ind:state_trajectory_inds(m));
        state_trajectory_start_ind = state_trajectory_inds(m) + 1;

        % Choose algorithms to try
        if pre_hold
            algorithms = algorithms_pre_hold;
        else
            algorithms = algorithms_post_hold;
        end

        for i = 1:length(algorithms)
            intermediate_points = algorithms{i}.intermediate_points;
            algorithm = algorithms{i}.algorithm;
            algorithm_name = algorithms{i}.algorithm_name;
            check_duration = algorithms{i}.check_duration;
            check_relative_error = algorithms{i}.check_relative_error;
    
            disp(['Trying with ', num2str(intermediate_points), ' intermediate points per mode using ''', algorithm_name, ''' algorithm.']);
    
            % Calculate the number of intermediate points for each mode.
            num_of_intermediate_points_per_mode = calculate_num_of_intermediate_points_per_mode(wavelet, intermediate_points);
    
            % Sample the state trajectory using the waveform.
            sampling_points = obj.sample_state_trajectory_by_waveform(wavelet_state_trajectory, wavelet, num_of_intermediate_points_per_mode);
            
            % Set mode position info
            if m == 1
                position = 1;
            elseif m == length(modes)
                position = 3;
            else
                position = 2;
            end

            for j = 1:length(sampling_points)
                sampling_points(j).mode_info.position = position;
            end

            %struct('current_mode', current_mode, 'mode_index', i, 'segment_index', j, 'is_last_segment', is_last_segment);

            % Approximate the waveform.
            [approximated_waveform, relative_errors] = obj.approximate(sampling_points, algorithm, initial_state);

            % Check that all waveform phases are ok, that is, they have a long enough duration.
            waveform_durations_ok = are_waveform_durations_ok(approximated_waveform, relative_errors);
    
            % Check that all waveform phases are ok, that is, they have a small enough relative error.
            relative_errors_ok = are_relative_errors_ok(relative_errors);

            % Plot solution
            %current_approximated_state_trajectory = obj.generate_state_trajectory_from_waveform(actual_voltage,[aggregated_approximated_waveform, approximated_waveform]);
            %obj.plot_state_trajectories(state_trajectory, current_approximated_state_trajectory, sampling_points);
            
    
            if (~check_duration || waveform_durations_ok) && (~check_relative_error || relative_errors_ok)
                success = true;
                disp('  The approximation was successful.');
    
                break;
            end
            disp('  The approximation failed.');
        end
        aggregated_approximated_waveform = [aggregated_approximated_waveform, approximated_waveform];

        if strcmp(modes(m),'h')
            pre_hold = 0;
        else
            pre_hold = 1;
        end

        % Update the initial state for the next wavelet iteration
        initial_state = obj.apply_waveform_to_state(initial_state, approximated_waveform);
    end


    function num_of_intermediate_points_per_mode = calculate_num_of_intermediate_points_per_mode(target_waveform, num_of_intermediate_points_for_rising_and_falling_modes)
        % Create number of intermediate points for each mode by using as many as possible for falling and rising modes,
        % and always zero for hold modes.
        num_of_intermediate_points_per_mode = zeros(1, length(target_waveform));
        for i = 1:length(target_waveform)
            if target_waveform(i).mode == 'f' || target_waveform(i).mode == 'r'
                num_of_intermediate_points_per_mode(i) = num_of_intermediate_points_for_rising_and_falling_modes;
            end
        end
    end

    % Define waveform durations to be ok if all phases have a duration longer than the threshold,
    % except initial and final phases if they are 'hold'.
    function is_ok = are_waveform_durations_ok(waveform, relative_errors)
        is_ok = true;
        for i = 1:length(waveform)
            % For first and last modes, check that if they are not 'hold', they have a duration longer than the threshold.
            first_and_last_mode_not_ok = (i == 1 || i == length(waveform)) && waveform(i).mode ~= 'h' && waveform(i).duration < obj.minimum_mode_duration;

            % For other modes, check that they have a duration longer than the threshold.
            other_mode_not_ok = i ~= 1 && i ~= length(waveform) && waveform(i).duration < obj.minimum_mode_duration;

            if first_and_last_mode_not_ok || other_mode_not_ok
                disp(['  Duration ', num2str(waveform(i).duration * 1e6), ' us of mode ', waveform(i).mode, ' at sampling point ', num2str(i), ' is too short.']);
                is_ok = false;
                break;
            end
        end
    end

    % Define waveform phases to be ok if all relative errors are smaller than the threshold.
    function is_ok = are_relative_errors_ok(relative_errors)
        is_ok = true;
        for i = 1:length(relative_errors)
            if relative_errors(i) > relative_error_threshold
                disp(['  Relative error ', num2str(100 * relative_errors(i)), '% of current at sampling point ', num2str(i), ' is too high.']);
                is_ok = false;
                break;
            end
        end
    end
end
