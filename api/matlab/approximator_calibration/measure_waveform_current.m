
%% Initialize api

channel_count = 5;
api = MTMSApi(channel_count);
api.start_device();
api.start_session()

%% Initialize mTMS toolkit

addpath(genpath("/home/mtms/mtms/api/matlab/mTMS_toolkit"))

mtms_tk = mTMS_toolkit(api,[]);

%% Initialize the scope
rosinit
%rosshutdown
% Then connect to this master ros from Windows PC

% Share parameters within ros
ptree = rosparam;
set(ptree,'max_current',0);
set(ptree,'collect',0);
set(ptree,'activate_scope',0);
set(ptree, 'scope_data', {});


%%

% Set reference pulse durations.
% reference_durations =  [60,40,140,10,60.5; ...
%                         60,40,140,10,59.6;...
%                         60,40,140,10,61.7;...
%                         60,40,140,10,61.6;...
%                         60,40,140,10,70.3]*1e-6;
custom_wfs = [];
for i = 1:5
    custom_wfs{i}(1).mode = 'r';
    custom_wfs{i}(1).duration = 0.75e-3;
    custom_wfs{i}(2).mode = 'f';
    custom_wfs{i}(2).duration = 0.75e-3;
end

%% Run this before stimulating and then run segment on other PC
set(ptree,'collect',0);
set(ptree,'max_current',0);
set(ptree,'activate_scope',1);
disp('Setup complete.')

%% Define PWM pulses
channel = 5;
file_id = sprintf("testdata_origapprox_coil%i",channel);

%%

test_load_voltages = [100];%[600,700,800,900,1000,1100,1200,1300,1400,1499];
data = struct([]);

f=figure(69);clf

for i = 1:length(test_load_voltages)

    reference_load_voltages = zeros(1,5);
    reference_load_voltages(channel) = test_load_voltages(i);

    load_voltages = zeros(1,5);
    load_voltages(channel) = test_load_voltages(i);


    % Define reference waveforms which will be approximated with PWM
    %reference_waveforms = mtms_tk.get_monophasic_reference_waveforms();
    reference_waveforms = custom_wfs;

    % Generate PWM waveforms for the pulse. (may take ~10 s)
    [PWM_waveforms,~,PWM_state_trajectory,ref_state_trajectory] = mtms_tk.generate_PWM_waveforms(reference_load_voltages, load_voltages, reference_waveforms);

    max_current = max(abs([PWM_state_trajectory{channel}.I_coil]));

    % Generate the pulse structure, and use the waveforms argument to specify the custom waveform.
    PWM_pulse_structure = mtms_tk.generate_pulse_structure(load_voltages, ...
        waveforms = PWM_waveforms, ...
        trigger_out=2);

    % Generate reference pulse structure
    ref_pulse_structure = mtms_tk.generate_pulse_structure(reference_load_voltages, ...
        waveforms = reference_waveforms, ...
        trigger_out=2);
    
    %disp("Waiting for button press...")
    waitforbuttonpress
    %drawnow

    figure(f); % Do this to set figure as active for applying plots
    nexttile; hold on

    %%% Reference pulse
    set(ptree,'max_current',max_current);
    mtms_tk.stimulate(ref_pulse_structure);

    set(ptree,"/collect",1)
    while get(ptree,'collect'); end
    obtained_scope = get(ptree, "scope_data");
    scope_data.time_pulse = cell2mat(obtained_scope{1});
    scope_data.pulse = cell2mat(obtained_scope{2});
    scope_data.time_trigger = cell2mat(obtained_scope{3});
    scope_data.trigger = cell2mat(obtained_scope{4});

    data(i).ref = scope_data;
    clear scope_data
    % 
    % %%% PWM pulse
    % set(ptree,'max_current',max_current);
    % 
    % % HACK
    % mtms_tk.stimulate(ref_pulse_structure);
    % 
    % set(ptree,"/collect",1)
    % 
    % while get(ptree,'collect'); end
    % 
    % obtained_scope = get(ptree, "scope_data");
    % scope_data.time_pulse = cell2mat(obtained_scope{1});
    % scope_data.pulse = cell2mat(obtained_scope{2});
    % scope_data.time_trigger = cell2mat(obtained_scope{3});
    % scope_data.trigger = cell2mat(obtained_scope{4});
    % 
    % data(i).pwm = scope_data;

    %%% Plot measured vs estimated
    est_current_ref = [ref_state_trajectory{channel}.I_coil];
    time_ref = 0:mtms_tk.approximator.time_resolution:(length(est_current_ref)-1)*mtms_tk.approximator.time_resolution;

    % est_current_pwm = [PWM_state_trajectory{channel}.I_coil];
    % time_pwm = 0:mtms_tk.approximator.time_resolution:(length(est_current_pwm)-1)*mtms_tk.approximator.time_resolution;

    % baseline_dur = 2e-6; % seconds
    % baseline_length = find(scope_data.time_pulse)
    i_offset = mean(data(i).ref.pulse(1:100));
    p1 = plot(data(i).ref.time_pulse,data(i).ref.pulse-i_offset,'LineWidth',2);
    p2 = plot(time_ref+3e-6,est_current_ref,'LineWidth',2);
    % p3 = plot(data(i).pwm.time_pulse,data(i).pwm.pulse-i_offset,'LineWidth',2);
    % p4 = plot(time_pwm+3.5e-6,est_current_pwm,'LineWidth',2);

    %legend([p1,p2,p3,p4],"Meas. ref","Est. ref", "Meas. PWM", "Est. PWM")

    % %%% Plot trigger vs measured
    % nexttile;hold on
    % p5 = plot(data(i).ref.time_trigger,data(i).ref.trigger,'LineWidth',2);
    % p6 = plot(data(i).ref.time_pulse,data(i).ref.pulse,'LineWidth',2);
    % p7 = plot(data(i).pwm.time_trigger,data(i).pwm.trigger,'LineWidth',2);
    % p8 = plot(data(i).pwm.time_pulse,data(i).pwm.pulse,'LineWidth',2);
    % 
    % legend([p5,p6,p7,p8],"Ref trig", "Ref", "Pwm trig", "PWM")

    data(i).ref.waveform = reference_waveforms;
    data(i).voltages = reference_load_voltages;
    data(i).pwm.waveform = PWM_waveforms;
    data(i).time = time_ref;
    data(i).ref.est_pulse = est_current_ref;
    %data(i).pwm.est_pulse = est_current_pwm;

end
mtms_tk.full_discharge()
save(file_id + "_" + string(datetime('now','Format','yyyyMMddHHmmss'))+ ".mat", 'data')

%% Calcualte errors
max_est_currents_ref = [];
max_meas_currents_ref = [];
errors_ref = [];

max_est_currents_pwm = [];
max_meas_currents_pwm = [];
errors_pwm = [];

for i = 1:length(test_load_voltages)
    max_est_currents_ref(i) = max(abs(data(i).ref.est_pulse));
    max_meas_currents_ref(i) = max(abs(data(i).ref.pulse));

    max_est_currents_pwm(i) = max(abs(data(i).pwm.est_pulse));
    max_meas_currents_pwm(i) = max(abs(data(i).pwm.pulse));

end

errors_ref = (max_est_currents_ref ./ max_meas_currents_ref) * 100;
errors_pwm = (max_est_currents_pwm ./ max_meas_currents_pwm) * 100;
errors_ref2pwm = (max_meas_currents_pwm ./ max_meas_currents_ref) * 100;

figure; hold on
pp1 = plot(errors_ref);
pp2 = plot(errors_pwm);
pp3 = plot(errors_ref2pwm);
legend([pp1,pp2,pp3], "Est / Meas: REF", "Est / Meas: PWM", "PWM / REF: Meas")

%%
set(ptree,'/activate_scope',0)
mtms_tk.full_discharge()
%%
