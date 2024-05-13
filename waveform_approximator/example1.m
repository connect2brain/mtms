% Example: Approximate a waveform using the 'alternating hold' algorithm.
%
% This script demonstrates how to approximate a waveform using the alternating hold algorithm.

clear all

solutions_filename = 'solutions_five_coil_set.mat';

time_resolution = 0.01e-6;

% Create the approximator object.
approximator = WaveformApproximator(solutions_filename, time_resolution);

% Select the coil.
approximator.select_coil(1);

% Select the algorithm.
algorithm = @approximator.algorithm_alternating_hold;

% Set the actual and target voltages.
actual_voltage = 1500;

% Note: If target voltage is very low, the approximation might contain modes with too short a duration;
%       checking the durations is the responsibility of the user.
target_voltage = 1000;


% Create a simple target waveform.
target_waveform = struct( ...
    'mode', {'f', 'h', 'r'}, ...
    'duration', {60 * 1e-6, 30 * 1e-6, 37 * 1e-6}, ...
    'num_of_intermediate_points', {3, 0, 3} ...
);

% Generate the state trajectory for the target waveform.
state_trajectory = approximator.generate_state_trajectory_from_waveform(target_voltage, target_waveform);

% Sample the state trajectory using the waveform.
sampling_points = approximator.sample_state_trajectory_by_waveform(state_trajectory, target_waveform);

% Approximate the waveform.
approximated_waveform = approximator.approximate(actual_voltage, sampling_points, algorithm);

% Generate the state trajectory for the approximated waveform.
approximated_state_trajectory = approximator.generate_state_trajectory_from_waveform(actual_voltage, approximated_waveform);

% Plot the state trajectories.
approximator.plot_state_trajectories(state_trajectory, approximated_state_trajectory, sampling_points)
