clear all

% A .mat file containing the precalculated solutions for each coil.
solutions_file = 'solutions_five_coil_set.mat';

% 0.1e-6 is faster but less accurate. Using 0.01e-6 for now.
resolution = 0.01e-6;

% Create the approximator object
approximator = WaveformApproximator(solutions_file, resolution);

% Select the coil
approximator.select_coil(1);

%% Example: Approximate a waveform with PWM

% Set the actual voltage and the voltage to approximate
actual_voltage = 1500;

% Note: If approximated voltage is very low, the approximation won't be feasible. No error checking is done for now.
target_voltage = 1000;

% Create monotrapezoid waveform
target_waveform.modes = 'fhrh';
target_waveform.durations = [60, 30, 37, 5.57] * 1e-6;

% The number of approximation steps for each mode
steps = [2, 1, 2, 1];

% Approximate the waveform
approximated_waveform = approximator.approximate(actual_voltage, target_voltage, target_waveform, steps);

% Calculate the final voltage for both waveforms
final_voltage_original = approximator.calculate_final_voltage(target_voltage, target_waveform);
final_voltage_approximated = approximator.calculate_final_voltage(actual_voltage, approximated_waveform);

%% Example: Calculate loss of energy during the pulse

% Calculate the loss of energy for both waveforms
e = @(initial_voltage, final_voltage) 0.5 * 1e-3 * (initial_voltage^2 - final_voltage^2);

disp('Energy loss (J) for the approximated waveform:')
disp(e(actual_voltage, final_voltage_approximated))

disp('Energy loss (J) for the original waveform:')
disp(e(target_voltage, final_voltage_original))

%% Example: Plot timecourse of current during the pulse

% Calculate the timecourse for the original waveform and the approximated waveform
timecourse_original = approximator.calculate_timecourse(target_voltage, target_waveform);
timecourse_approximated = approximator.calculate_timecourse(actual_voltage, approximated_waveform);

% Plot the timecourse of coil current for both waveforms
figure
hold on
plot(timecourse_original.I_coil)
plot(timecourse_approximated.I_coil)
set(gcf, 'color', 'w')
