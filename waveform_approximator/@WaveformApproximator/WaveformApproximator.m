classdef WaveformApproximator < handle

    properties (SetAccess = public, GetAccess = public)
        resolution;                     % resolution of calculation
        solutions;
        functions;
        metadata;
    end

    properties (SetAccess = private, GetAccess = public)

    end

    % Public methods for calculating and visualizing waveforms, updating
    % the reference
    methods (Access = public)
        % Approximator constructor
        % input:
        %   @solutions_file: file containing phase functions
        %   @resolution: system resolution
        function obj = WaveformApproximator(solutions_file, resolution)
            obj.resolution = resolution;
            obj.solutions = load(solutions_file);
        end

        function select_coil(obj, index)
            assert (index > 0, "Index must be greater than 0");
            assert (index <= length(obj.solutions.sols), "Index must be less than or equal to the number of solutions");

            obj.functions = obj.solutions.sols(index).f;
            obj.metadata = obj.solutions.sols(index).metadata;
        end

        % Calculate a timecourse from a command sequence and an initial system voltage
        % input:
        %   @voltage: initial voltage
        %   @waveform: a waveform struct with the following fields:
        %       @modes: the mode of each waveform phase as a string, e.g.,'fhrh' for a 4-phase waveform
        %       @durations: a list of durations for each mode
        % output:
        %   @timecourse: resulting timecourse, including the fields V_c, V_s1, V_s2, ..., I_z_R.
        timecourse = calculate_timecourse(obj, voltage, waveform);

        % Approximate the reference waveform with initial voltage and step information
        % input:
        %   @actual_voltage: the initial voltage to do the approximation with
        %   @target_voltage: the voltage to approximate to
        %   @waveform: a waveform struct with the following fields:
        %       @modes: the mode of each waveform phase as a string, e.g.,'fhrh' for a 4-phase waveform
        %       @durations: a list of durations for each mode
        %   @steps: a list of the number of approximation steps for each mode
        % output:
        %   @waveform: a struct containing the approximate waveform, includes the following fields:
        %       @modes: the mode of each waveform phase as a string
        %       @durations: a list of durations for each mode
        waveform = approximate(obj, actual_voltage, target_voltage, waveform, steps);

        % Calculate the final voltage after a waveform is applied.
        % input:
        %   @voltage: initial voltage
        %   @waveform: a waveform struct with the following fields:
        %       @modes: the mode of each waveform phase as a string, e.g.,'fhrh' for a 4-phase waveform
        %       @durations: a list of durations for each mode
        final_voltage = calculate_final_voltage(obj, voltage, waveform);
    end

    % Private methods for generating approximations
    methods (Access = private)
        % Calculate all the parameters (V_c, ..., I_z_R) after applying a mode for a specified duration.
        % input:
        %   @initial_conditions: initial conditions; a struct including the fields V_c, ..., I_z_R
        %   @mode: mode as a character (either 'f', 'h', 'r', or 'a')
        %   @duration: duration of the mode
        parameters = calculate_step(obj, initial_conditions, mode, duration);

        % Calculate value of coil current after a specified time
        coil_current = calculate_coil_current(obj, initial_conditions, step_type, dt);

        % Generate initial conditions struct from a voltage
        % input:
        %   @voltage: initial voltage
        %   @len: length of each initial condition vector
        % output:
        %   @ic_struct: a generated struct, including the fields V_c, ..., I_z_R. Each field is a vector of length 'len'.
        ic_struct = generate_initial_conditions(obj, voltage, len);

        % Extract initial conditions timepoint from an initial conditions struct by index
        initial_conditions = get_initial_conditions_by_index(obj, ic_struct, index);

        % Get crossing points for the approximator to follow
        % input:
        %   @waveform: a waveform struct with the following fields:
        %     @modes: the mode of each waveform phase as a string, e.g.,'fhrh' for a 4-phase waveform
        %     @durations: the duration of each waveform phase, e.g., [60, 30, 37, 5.57] * 1e-6
        %   @steps: the number of approximation steps for each waveform phase, e.g., [2, 1, 2, 1]
        % output:
        %   @crossing_points: points (indices) where the approximate waveform crosses the reference waveform
        crossing_points = get_crossing_points(obj, waveform, steps);
    end
end