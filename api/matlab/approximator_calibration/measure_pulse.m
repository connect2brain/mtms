function scope_data = measure_pulse(waveforms,load_voltage,mtms_tk,ptree)

%set(ptree,'collect',0);
%set(ptree,'max_current',0);
%set(ptree,'activate_scope',1);
%disp('Setup complete.')

pulse_structure = mtms_tk.generate_pulse_structure(load_voltage, ...
    waveforms = waveforms, ...
    trigger_out=2);

% Estimate maximum current
state_trajectory = mtms_tk.approximator.generate_state_trajectory_from_waveform(abs(load_voltage),waveforms{1});
max_current = max(abs([state_trajectory.I_coil]));

% Instruct to adjust scope
set(ptree,'max_current',max_current)
set(ptree,'max_voltage',abs(load_voltage))
set(ptree,'adjust_scope',1)
while get(ptree,'adjust_scope'); end

% Execute pulse
mtms_tk.stimulate(pulse_structure);

% Collect adata
set(ptree,"/collect",1)
while get(ptree,'collect'); end
obtained_scope = get(ptree, "scope_data");

% Parse data
scope_data.time_current = cell2mat(obtained_scope{1});
scope_data.current = cell2mat(obtained_scope{2});

scope_data.time_voltage = cell2mat(obtained_scope{3});
scope_data.voltage = cell2mat(obtained_scope{4});

scope_data.time_trigger = cell2mat(obtained_scope{5});
scope_data.trigger = cell2mat(obtained_scope{6});


end