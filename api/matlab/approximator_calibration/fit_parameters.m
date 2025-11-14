%% Example: Find solutions and store them in a .mat file.

% Load pulse data
%load('1_10_5ch_coil_1.mat');
load("testdata_origapprox_coil5.mat");

for i = 1:length(data)
    pulse_data(i).i_meas = data(i).ref.pulse';
    pulse_data(i).t_meas = data(i).ref.time_pulse';
    pulse_data(i).voltage.before_pulse = data(i).voltages(5);
end

%%

addpath("/home/mtms/mtms/ros2_ws/src/mtms_packages/targeting/waveform_approximator/waveform_approximator/solutions/")

% Find system resistance and inductance during rising phase
my_params = [];

for i = 1:length(data)

    [R_rise, L_rise, R_hold, L_hold] = characterize_coil(pulse_data(i), 'true', 'true');
my_params(i,:) = [R_rise, L_rise, R_hold, L_hold];
end

%% Assemble a solutions file from characterized values
%{
coil_parameters.R_rise = [104, 107, 100, 100, 65]*1e-3;
coil_parameters.R_hold = [101, 104, 97, 97, 62]*1e-3;
coil_parameters.L_rise = [15.3, 15.2, 15.6, 15.5, 17.7]*1e-6;
coil_parameters.L_hold = [15.1, 15.0, 15.4, 15.3, 17.5]*1e-6;
coil_parameters.C      = [1, 1, 1, 1, 1]*1e-3;
%}

coil_parameters.R_rise = R_rise;
coil_parameters.L_rise = L_rise;
coil_parameters.R_hold = R_hold;
coil_parameters.L_hold = L_hold;
coil_parameters.C = 1e-3;

% Calculate circuit solutions
sols = calculate_solutions(coil_parameters);

% HACK
sols = repmat(sols,1,5);

% Save solutions file
save('test_solutions1_V500.mat', 'sols');
