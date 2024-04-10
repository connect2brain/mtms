% Calculate the resulting timecourse of the circuit with the given waveform and initial voltage.
function timecourse = calculate_timecourse(obj, voltage, waveform)

    %t_int = 0.4e-6;
    t_int = 0;

    rounding_factor = fix(abs(log10(abs(1e-6/obj.resolution))));
    waveform.durations = round(waveform.durations/1e-6, rounding_factor)*1e-6;
    num_elements = fix(sum(waveform.durations)/obj.resolution); % was: round( ... +1 )

    V_c = zeros(1, num_elements);       V_c(1) = voltage;
    V_s1 = zeros(1, num_elements);      V_s1(1) = 0;
    V_s2 = zeros(1, num_elements);      V_s2(1) = voltage;
    V_s3 = zeros(1, num_elements);      V_s3(1) = 0;
    V_s4 = zeros(1, num_elements);      V_s4(1) = voltage;
    I_coil = zeros(1, num_elements);    I_coil(1) = 0;
    I_tw = zeros(1, num_elements);      I_tw(1) = 0;
    I_bar_U = zeros(1, num_elements);   I_bar_U(1) = 0;
    I_bar_L = zeros(1, num_elements);   I_bar_L(1) = 0;
    I_z_L = zeros(1, num_elements);     I_z_L(1) = 0;
    I_z_R = zeros(1, num_elements);     I_z_R(1) = 0;

    % Maximum diode conduction duration: 5e-6
    cond_max = 5e-6;
    I_d = zeros(1, round(cond_max/obj.resolution));

    % Tracking index
    idx = 1;
    for i = 1:length(waveform.modes)

        % R/F changed!
        if waveform.modes(i) == 'f'
            % rise, diode conduction
            %
            conduction_dur = cond_max;
            t = obj.resolution:obj.resolution:conduction_dur;

            I_d = obj.functions.i_rise_diode_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            rev_index = find(I_d < 0, 1);
            conduction_dur = obj.resolution*(rev_index-1);

            if conduction_dur ~= 0
                t = obj.resolution:obj.resolution:conduction_dur;
                size = length(t);
                V_c(idx+1:idx+size)     = obj.functions.v_rise_cap_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s1(idx+1:idx+size)    = obj.functions.v_rise_sn1_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s2(idx+1:idx+size)    = obj.functions.v_rise_sn2_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s3(idx+1:idx+size)    = obj.functions.v_rise_sn3_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s4(idx+1:idx+size)    = obj.functions.v_rise_sn4_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

                I_coil(idx+1:idx+size)  = obj.functions.i_rise_coil_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_tw(idx+1:idx+size)    = obj.functions.i_rise_tw_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_bar_U(idx+1:idx+size) = obj.functions.i_rise_barU_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_bar_L(idx+1:idx+size) = obj.functions.i_rise_barL_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_z_L(idx+1:idx+size)   = obj.functions.i_rise_zL_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_z_R(idx+1:idx+size)   = obj.functions.i_rise_zR_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

                idx = idx+size;
            end
            %}

            % interleaved circuit
            %{
            t = obj.resolution:obj.resolution:t_int;
            size = length(t);

            V_c(idx+1:idx+size)     = obj.functions.v_int_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s1(idx+1:idx+size)    = obj.functions.v_int_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s2(idx+1:idx+size)    = obj.functions.v_int_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s3(idx+1:idx+size)    = obj.functions.v_int_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s4(idx+1:idx+size)    = obj.functions.v_int_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            I_coil(idx+1:idx+size)  = obj.functions.i_int_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_tw(idx+1:idx+size)    = obj.functions.i_int_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_U(idx+1:idx+size) = obj.functions.i_int_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_L(idx+1:idx+size) = obj.functions.i_int_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_L(idx+1:idx+size)   = obj.functions.i_int_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_R(idx+1:idx+size)   = obj.functions.i_int_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            idx = idx+size;
            %}

            % rise
            t = obj.resolution:obj.resolution:waveform.durations(i)-conduction_dur-t_int; % note: t_int
            size = length(t);

            V_c(idx+1:idx+size)     = obj.functions.v_rise_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s1(idx+1:idx+size)    = obj.functions.v_rise_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s2(idx+1:idx+size)    = obj.functions.v_rise_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s3(idx+1:idx+size)    = obj.functions.v_rise_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s4(idx+1:idx+size)    = obj.functions.v_rise_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            I_coil(idx+1:idx+size)  = obj.functions.i_rise_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_tw(idx+1:idx+size)    = obj.functions.i_rise_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_U(idx+1:idx+size) = obj.functions.i_rise_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_L(idx+1:idx+size) = obj.functions.i_rise_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_L(idx+1:idx+size)   = obj.functions.i_rise_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_R(idx+1:idx+size)   = obj.functions.i_rise_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            idx = idx+size;


        elseif waveform.modes(i) == 'h'
            % interleaved circuit
            %{
            if i > 1
                t = obj.resolution:obj.resolution:t_int;
                size = length(t);

                if waveform.modes(i-1) == 'f' %int
                    V_c(idx+1:idx+size)     = obj.functions.v_int_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s1(idx+1:idx+size)    = obj.functions.v_int_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s2(idx+1:idx+size)    = obj.functions.v_int_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s3(idx+1:idx+size)    = obj.functions.v_int_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s4(idx+1:idx+size)    = obj.functions.v_int_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

                    I_coil(idx+1:idx+size)  = obj.functions.i_int_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_tw(idx+1:idx+size)    = obj.functions.i_int_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_bar_U(idx+1:idx+size) = obj.functions.i_int_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_bar_L(idx+1:idx+size) = obj.functions.i_int_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_z_L(idx+1:idx+size)   = obj.functions.i_int_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_z_R(idx+1:idx+size)   = obj.functions.i_int_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                elseif waveform.modes(i-1) == 'r' %int_rev
                    V_c(idx+1:idx+size)     = obj.functions.v_int_rev_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s1(idx+1:idx+size)    = obj.functions.v_int_rev_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s2(idx+1:idx+size)    = obj.functions.v_int_rev_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s3(idx+1:idx+size)    = obj.functions.v_int_rev_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    V_s4(idx+1:idx+size)    = obj.functions.v_int_rev_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

                    I_coil(idx+1:idx+size)  = obj.functions.i_int_rev_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_tw(idx+1:idx+size)    = obj.functions.i_int_rev_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_bar_U(idx+1:idx+size) = obj.functions.i_int_rev_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_bar_L(idx+1:idx+size) = obj.functions.i_int_rev_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_z_L(idx+1:idx+size)   = obj.functions.i_int_rev_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                    I_z_R(idx+1:idx+size)   = obj.functions.i_int_rev_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                end

                idx = idx+size;
            end
            %}

            % top hold
            t = obj.resolution:obj.resolution:waveform.durations(i)-t_int;
            size = length(t);
            V_c(idx+1:idx+size)     = obj.functions.v_top_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s1(idx+1:idx+size)    = obj.functions.v_top_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s2(idx+1:idx+size)    = obj.functions.v_top_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s3(idx+1:idx+size)    = obj.functions.v_top_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s4(idx+1:idx+size)    = obj.functions.v_top_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            I_coil(idx+1:idx+size)  = obj.functions.i_top_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_tw(idx+1:idx+size)    = obj.functions.i_top_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_U(idx+1:idx+size) = obj.functions.i_top_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_L(idx+1:idx+size) = obj.functions.i_top_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_L(idx+1:idx+size)   = obj.functions.i_top_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_R(idx+1:idx+size)   = obj.functions.i_top_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            idx = idx+size;

        elseif waveform.modes(i) == 'a'

            % bottom hold
            t = obj.resolution:obj.resolution:waveform.durations(i);
            size = length(t);
            V_c(idx+1:idx+size)     = obj.functions.v_bot_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s1(idx+1:idx+size)    = obj.functions.v_bot_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s2(idx+1:idx+size)    = obj.functions.v_bot_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s3(idx+1:idx+size)    = obj.functions.v_bot_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s4(idx+1:idx+size)    = obj.functions.v_bot_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            I_coil(idx+1:idx+size)  = obj.functions.i_bot_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_tw(idx+1:idx+size)    = obj.functions.i_bot_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_U(idx+1:idx+size) = obj.functions.i_bot_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_L(idx+1:idx+size) = obj.functions.i_bot_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_L(idx+1:idx+size)   = obj.functions.i_bot_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_R(idx+1:idx+size)   = obj.functions.i_bot_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            idx = idx+size;

        elseif waveform.modes(i) == 'r'
            % fall, diode conduction
            %{
            conduction_dur = cond_max;
            t = obj.resolution:obj.resolution:conduction_dur;

            I_d = obj.functions.i_fall_diode_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            rev_index = find(I_d < 0, 1);
            conduction_dur = obj.resolution*(rev_index-1);

            if conduction_dur ~= 0
                t = obj.resolution:obj.resolution:conduction_dur;
                size = length(t);
                V_c(idx+1:idx+size)     = obj.functions.v_fall_cap_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s1(idx+1:idx+size)    = obj.functions.v_fall_sn1_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s2(idx+1:idx+size)    = obj.functions.v_fall_sn2_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s3(idx+1:idx+size)    = obj.functions.v_fall_sn3_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                V_s4(idx+1:idx+size)    = obj.functions.v_fall_sn4_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

                I_coil(idx+1:idx+size)  = obj.functions.i_fall_coil_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_tw(idx+1:idx+size)    = obj.functions.i_fall_tw_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_bar_U(idx+1:idx+size) = obj.functions.i_fall_barU_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_bar_L(idx+1:idx+size) = obj.functions.i_fall_barL_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_z_L(idx+1:idx+size)   = obj.functions.i_fall_zL_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
                I_z_R(idx+1:idx+size)   = obj.functions.i_fall_zR_d_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

                idx = idx+size;
            end
            %}

            % interleaved circuit
            %{
            t = obj.resolution:obj.resolution:t_int;
            size = length(t);

            V_c(idx+1:idx+size)     = obj.functions.v_int_rev_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s1(idx+1:idx+size)    = obj.functions.v_int_rev_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s2(idx+1:idx+size)    = obj.functions.v_int_rev_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s3(idx+1:idx+size)    = obj.functions.v_int_rev_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s4(idx+1:idx+size)    = obj.functions.v_int_rev_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            I_coil(idx+1:idx+size)  = obj.functions.i_int_rev_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_tw(idx+1:idx+size)    = obj.functions.i_int_rev_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_U(idx+1:idx+size) = obj.functions.i_int_rev_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_L(idx+1:idx+size) = obj.functions.i_int_rev_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_L(idx+1:idx+size)   = obj.functions.i_int_rev_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_R(idx+1:idx+size)   = obj.functions.i_int_rev_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            idx = idx+size;
            %}

            % fall
            t = obj.resolution:obj.resolution:waveform.durations(i);%-conduction_dur;
            size = length(t);
            V_c(idx+1:idx+size)     = obj.functions.v_fall_cap_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s1(idx+1:idx+size)    = obj.functions.v_fall_sn1_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s2(idx+1:idx+size)    = obj.functions.v_fall_sn2_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s3(idx+1:idx+size)    = obj.functions.v_fall_sn3_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            V_s4(idx+1:idx+size)    = obj.functions.v_fall_sn4_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            I_coil(idx+1:idx+size)  = obj.functions.i_fall_coil_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_tw(idx+1:idx+size)    = obj.functions.i_fall_tw_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_U(idx+1:idx+size) = obj.functions.i_fall_barU_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_bar_L(idx+1:idx+size) = obj.functions.i_fall_barL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_L(idx+1:idx+size)   = obj.functions.i_fall_zL_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);
            I_z_R(idx+1:idx+size)   = obj.functions.i_fall_zR_simp(I_bar_L(idx), I_bar_U(idx), I_coil(idx), I_tw(idx), I_z_L(idx), I_z_R(idx), V_c(idx), V_s1(idx), V_s2(idx), V_s3(idx), V_s4(idx), t);

            idx = idx+size;
        end
    end

    timecourse.V_c      = V_c;
    timecourse.V_s1     = V_s1;
    timecourse.V_s2     = V_s2;
    timecourse.V_s3     = V_s3;
    timecourse.V_s4     = V_s4;
    timecourse.I_coil   = I_coil;
    timecourse.I_tw     = I_tw;
    timecourse.I_bar_U  = I_bar_U;
    timecourse.I_bar_L  = I_bar_L;
    timecourse.I_z_L    = I_z_L;
    timecourse.I_z_R    = I_z_R;
end