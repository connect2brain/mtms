function ic = generate_initial_conditions(obj, voltage, len)
    ic.V_c = ones(1, len)*voltage;
    ic.V_s1 = zeros(1, len);
    ic.V_s2 = ones(1, len)*voltage;
    ic.V_s3 = zeros(1, len);
    ic.V_s4 = ones(1, len)*voltage;
    ic.I_coil = zeros(1, len);
    ic.I_tw = zeros(1, len);
    ic.I_bar_U = zeros(1, len);
    ic.I_bar_L = zeros(1, len);
    ic.I_z_L = zeros(1, len);
    ic.I_z_R = zeros(1, len);
end