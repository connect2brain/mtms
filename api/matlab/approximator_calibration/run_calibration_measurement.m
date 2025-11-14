% Script for recording data for approximator model calibration.
%
% Setup oscilloscope for 3-channel capture:
%   Chn 1: Coil current with Rogowski
%   Chn 2: Coil voltage with differential probe
%   Chn 3: Trigger out signal from the mTMS cabinet
%
% One channel is used to measure all coils. This channel is specified in
% the mtms_tk.channels_in_use, which is set to 4 (channel 5)..
%
% Attach coil cables to this channel and change the 'coil_index' below
% accordingly. Do the measurement will all coils one-by-one.

%%

coil_index = 4; % CHANGE HERE

%%
filename = sprintf("pulse_data_coil%i.mat",coil_index);
if exist(filename,"file")
    error("Measurement file already exists!")
end

%%

channel_count = 6;
api = MTMSApi(channel_count);
api.start_device
api.start_session

coil_file = "/home/mtms/mtms/api/matlab/approximator_calibration/coil_specs_calibration.csv";
mtms_tk = mTMS_toolkit(api,coil_file,[]);

mtms_tk.channels_in_use = 5;    % Zero indexed!

%%

try
    rosnode list;  % Works only if ROS is initialized
    disp('ROS is already running.');
catch
    rosinit;
end
%rosshutdown
% Then connect to this master ros from Windows PC

% Share parameters within ros
ptree = rosparam;

set(ptree,'max_current',0);
set(ptree,'max_voltage',0);
set(ptree,'collect',0);
set(ptree,'warning',0)
set(ptree,'adjust_scope',0);
set(ptree, 'scope_data', {});

%% Define waveforms

all_waveforms = {};
% Rise and Fall
% custom_wfs = [];
% for i = 1:5
%     custom_wfs{i}(1).mode = 'r';
%     custom_wfs{i}(1).duration = 0.75e-3;
%     custom_wfs{i}(2).mode = 'f';
%     custom_wfs{i}(2).duration = 0.75e-3;
% end

all_waveforms{1} = mtms_tk.get_monophasic_reference_waveforms;

%% Define pulse voltages

all_load_voltages = 100:100:1500;

%%

num_waveforms = length(all_waveforms);
num_load_voltages = length(all_load_voltages);
num_repetitions = 2;

clear measurement

for i = 1:num_waveforms
    for j = 1:num_load_voltages
        for k = 1:num_repetitions
            measurement(i,j,k).data = measure_pulse(all_waveforms{i},all_load_voltages(j),mtms_tk,ptree);
            measurement(i,j,k).load_voltage = all_load_voltages(j);
            measurement(i,j,k).waveform = all_waveforms{i};
        end
    end
end

if exist(filename,"file")
    warning("Measurement file already exists!")
else
    save(filename,"measurement")
end

api.send_immediate_full_discharge_to_all_channels