% Example: Approximate a sinusoidal coil current function using the hold algorithm.

clear all

solutions_filename = 'solutions_five_coil_set.mat';

time_resolution = 0.01e-6;

% Create the approximator object.
approximator = WaveformApproximator(solutions_filename, time_resolution);

% Select the coil.
approximator.select_coil(1);

% Select the algorithm.
algorithm = @approximator.algorithm_hold;

% Set the actual and target voltages.
actual_voltage = 1500;

% Note: If target voltage is very low, the approximation might contain modes with too short a duration;
%       checking the durations is the responsibility of the user.
target_voltage = 1000;


% Generate a sinusoidal coil current function.
duration = 100 * 1e-6;
omega = 2 * pi / duration;
I_max = 1000;
coil_current_function = @(t) I_max * sin(omega * t);
num_of_intermediate_points = 20;

% Generate the state trajectory based on the coil current function.
state_trajectory = approximator.generate_state_trajectory_from_function(coil_current_function, duration);

% Sample the state trajectory uniformly.
sampling_points = approximator.sample_state_trajectory_uniformly(state_trajectory, num_of_intermediate_points);

% Approximate the waveform.
approximated_waveform = approximator.approximate(actual_voltage, sampling_points, algorithm);

% Generate the state trajectory for the approximated waveform
approximated_state_trajectory = approximator.generate_state_trajectory_from_waveform(actual_voltage, approximated_waveform);

% Plot the state trajectories.
approximator.plot_state_trajectories(state_trajectory, approximated_state_trajectory, sampling_points)
