classdef WaveformApproximator < handle

    properties (SetAccess = public, GetAccess = public)
        time_resolution;
        solutions;
        functions;
        metadata;
    end

    methods (Access = public)

        % Constructor for the waveform approximator.
        % input:
        %   @solutions_filename: filename of the file containing phase functions
        %   @time_resolution: time resolution for computing the state trajectories
        function obj = WaveformApproximator(solutions_file, time_resolution)
            obj.time_resolution = time_resolution;
            obj.solutions = load(solutions_file);
        end

        function select_coil(obj, index)
            assert (index > 0, "Index must be greater than 0");
            assert (index <= length(obj.solutions.sols), "Index must be less than or equal to the number of solutions");

            obj.functions = obj.solutions.sols(index).f;
            obj.metadata = obj.solutions.sols(index).metadata;
        end

        % Generate a state trajectory from an initial voltage and a waveform.
        % input:
        %   @voltage: initial voltage
        %   @waveform: an array of structs, each with the following fields:
        %       @mode: the mode of the waveform phase as a character, e.g., 'f' for falling
        %       @duration: the mode duration in seconds
        % output:
        %   @state_trajectory: an array of states, each state a struct, including the fields V_c, V_s1, V_s2, ..., I_z_R.
        state_trajectory = generate_state_trajectory_from_waveform(obj, voltage, waveform)

        % Generate a state trajectory from an arbitrary function and a duration.
        % input:
        %   @coil_current_function: function that returns the coil current at a given time in seconds
        %   @duration: duration for the state trajectory
        % output:
        %   @state_trajectory: an array of states, each state a struct, including the fields V_c, V_s1, V_s2, ..., I_z_R.
        state_trajectory = generate_state_trajectory_from_function(obj, coil_current_function, duration);

        % Generate sampling points from a state trajectory and a target waveform.
        % input:
        %   @state_trajectory: an array of states, each state a struct, including the fields V_c, V_s1, V_s2, ..., I_z_R.
        %   @target_waveform: an array of structs, each with the following fields:
        %       @mode: the mode of the waveform phase as a character, e.g., 'f' for falling
        %       @duration: the mode duration in seconds
        %       @num_of_intermediate_points: the number of intermediate points between two consecutive modes
        % output:
        %   @sampling_points: an array of structs, each with the following fields:
        %       @time: the time in seconds
        %       @index: the index of the state in the state trajectory
        %       @state: the state at the index
        %       @mode_info: a struct with the following fields:
        %           @next_mode: the mode of the next phase as a character, e.g., 'f' for falling
        %           @is_last: a boolean indicating if the mode is the last one
        %           @index: the index of the mode in the target waveform
        sampling_points = sample_state_trajectory_by_waveform(obj, state_trajectory, target_waveform)

        % Generate sampling points from a state trajectory and a number of intermediate points.
        % input:
        %   @state_trajectory: an array of states, each state a struct, including the fields V_c, V_s1, V_s2, ..., I_z_R.
        %   @num_of_sampling_points: the number of intermediate points between two consecutive modes
        % output:
        %   @sampling_points: an array of structs, each with the following fields:
        %       @time: the time in seconds
        %       @index: the index of the state in the state trajectory
        %       @state: the state at the index
        %       @mode_info: a struct with the following fields:
        %           @next_mode: the mode of the next phase as a character, e.g., 'f' for falling
        %           @is_last: a boolean indicating if the mode is the last one
        %           @index: the index of the mode in the target waveform
        sampling_points = sample_state_trajectory_uniformly(obj, state_trajectory, num_of_sampling_points)

        approximated_waveform = approximate(obj, actual_voltage, sampling_points, approximator, algorithm)

        % Estimate the final voltage after a waveform is applied.
        % input:
        %   @voltage: initial voltage
        %   @waveform: an array of structs, each with the following fields:
        %       @mode: the mode of the waveform phase as a character, e.g., 'f' for falling
        %       @duration: the mode duration in seconds
        % output:
        %   @final_voltage: the final voltage after the waveform is applied
        final_voltage = estimate_final_voltage(obj, voltage, waveform);

        plot_state_trajectories(obj, target_state_trajectory, approximated_state_trajectory, sampling_points)

        % Algorithms for approximating waveforms
        %
        % Usage, e.g.:
        %   algorithm = @approximator.algorithm_hold;
        %   approximated_waveform = approximator.approximate(actual_voltage, sampling_points, approximator, algorithm);

        % Approximate each sampling interval with a hold and either a falling or a rising mode.
        waveform = algorithm_hold(obj, parameter, total_duration, mode_info)

        % Approximate each sampling interval with a hold mode at the beginning and the end, and a falling or a rising
        % mode in between. The hold modes are of the same duration.
        waveform = algorithm_hold_both_ends(obj, parameter, total_duration, mode_info)

        % Approximate each sampling interval with a hold and either a falling or a rising mode. The order of the hold and
        % the falling/rising mode is alternating.
        waveform = algorithm_alternating_hold(obj, parameter, total_duration, mode_info)

        % Approximate each sampling interval with a hold, a falling mode, and a rising mode. The falling mode precedes
        % the rising mode if the target current is increasing, and vice versa.
        waveform = algorithm_falling_rising(obj, parameter, total_duration, mode_info)
    end
    
    methods (Access = private)
        initial_state = generate_initial_state(obj, voltage);

        final_state = apply_mode_to_state(obj, initial_state, mode, duration)
        final_state = apply_waveform_to_state(obj, initial_state, waveform)

        state_trajectory = generate_state_trajectory_from_mode(obj, initial_state, mode, duration, varargin)

        [parameter, error] = golden_section_search(obj, error_function, lower_bound, upper_bound)

        error = calculate_error(obj, parameter, algorithm, initial_state, target_state, total_duration, mode_info)        

        index = get_index_from_time(obj, time)
        time = get_state_trajectory_end_time(obj, state_trajectory)
    end
end
