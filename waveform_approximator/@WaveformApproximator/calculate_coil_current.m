function coil_current = calculate_coil_current(obj, initial_conditions, step_type, dt)

    V_c = initial_conditions.V_c;
    V_s1 = initial_conditions.V_s1;
    V_s2 = initial_conditions.V_s2;
    V_s3 = initial_conditions.V_s3;
    V_s4 = initial_conditions.V_s4;
    I_coil = initial_conditions.I_coil;
    I_tw = initial_conditions.I_tw;
    I_bar_U = initial_conditions.I_bar_U;
    I_bar_L = initial_conditions.I_bar_L;
    I_z_L = initial_conditions.I_z_L;
    I_z_R = initial_conditions.I_z_R;

    % Maximum diode conduction duration: 5e-6
    cond_max = 5e-6;
    I_d = zeros(1, round(cond_max/obj.resolution));

    % Calculate rising step
    if step_type == 'f'
        % rise, diode conduction
        conduction_dur = cond_max;
        t = obj.resolution:obj.resolution:conduction_dur;

        I_d = obj.functions.i_rise_diode_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, ...
                                              initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, ...
                                              initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, ...
                                              initial_conditions.V_s3, initial_conditions.V_s4, t);
        rev_index = find(I_d < 0, 1);
        conduction_dur = obj.resolution*(rev_index-1);

        if conduction_dur ~= 0
            t = conduction_dur;
            V_c_d = V_c; V_s1_d = V_s1; V_s2_d = V_s2; V_s3_d = V_s3; V_s4_d = V_s4;
            I_coil_d = I_coil; I_tw_d = I_tw; I_bar_U_d = I_bar_U; I_bar_L_d = I_bar_L; I_z_L_d = I_z_L; I_z_R_d = I_z_R;

            V_c     = obj.functions.v_rise_cap_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            V_s1    = obj.functions.v_rise_sn1_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            V_s2    = obj.functions.v_rise_sn2_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            V_s3    = obj.functions.v_rise_sn3_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            V_s4    = obj.functions.v_rise_sn4_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);

            I_coil  = obj.functions.i_rise_coil_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            I_tw    = obj.functions.i_rise_tw_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            I_bar_U = obj.functions.i_rise_barU_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            I_bar_L = obj.functions.i_rise_barL_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            I_z_L   = obj.functions.i_rise_zL_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
            I_z_R   = obj.functions.i_rise_zR_d_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        end

        % rise
        t = dt-conduction_dur;
        V_c_d = V_c; V_s1_d = V_s1; V_s2_d = V_s2; V_s3_d = V_s3; V_s4_d = V_s4;
        I_coil_d = I_coil; I_tw_d = I_tw; I_bar_U_d = I_bar_U; I_bar_L_d = I_bar_L; I_z_L_d = I_z_L; I_z_R_d = I_z_R;

        V_c     = obj.functions.v_rise_cap_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        V_s1    = obj.functions.v_rise_sn1_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        V_s2    = obj.functions.v_rise_sn2_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        V_s3    = obj.functions.v_rise_sn3_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        V_s4    = obj.functions.v_rise_sn4_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);

        I_coil  = obj.functions.i_rise_coil_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        I_tw    = obj.functions.i_rise_tw_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        I_bar_U = obj.functions.i_rise_barU_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        I_bar_L = obj.functions.i_rise_barL_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        I_z_L   = obj.functions.i_rise_zL_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);
        I_z_R   = obj.functions.i_rise_zR_simp(I_bar_L_d, I_bar_U_d, I_coil_d, I_tw_d, I_z_L_d, I_z_R_d, V_c_d, V_s1_d, V_s2_d, V_s3_d, V_s4_d, t);

    elseif step_type == 'h'
        % top hold
        V_c     = obj.functions.v_top_cap_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s1    = obj.functions.v_top_sn1_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s2    = obj.functions.v_top_sn2_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s3    = obj.functions.v_top_sn3_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s4    = obj.functions.v_top_sn4_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);

        I_coil  = obj.functions.i_top_coil_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_tw    = obj.functions.i_top_tw_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_bar_U = obj.functions.i_top_barU_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_bar_L = obj.functions.i_top_barL_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_z_L   = obj.functions.i_top_zL_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_z_R   = obj.functions.i_top_zR_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);

    elseif step_type == 'a'
        % bottom hold
        V_c     = obj.functions.v_bot_cap_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s1    = obj.functions.v_bot_sn1_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s2    = obj.functions.v_bot_sn2_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s3    = obj.functions.v_bot_sn3_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s4    = obj.functions.v_bot_sn4_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);

        I_coil  = obj.functions.i_bot_coil_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_tw    = obj.functions.i_bot_tw_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_bar_U = obj.functions.i_bot_barU_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_bar_L = obj.functions.i_bot_barL_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_z_L   = obj.functions.i_bot_zL_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_z_R   = obj.functions.i_bot_zR_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);

    elseif step_type == 'r'
        % fall
        V_c     = obj.functions.v_fall_cap_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s1    = obj.functions.v_fall_sn1_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s2    = obj.functions.v_fall_sn2_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s3    = obj.functions.v_fall_sn3_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        V_s4    = obj.functions.v_fall_sn4_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);

        I_coil  = obj.functions.i_fall_coil_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_tw    = obj.functions.i_fall_tw_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_bar_U = obj.functions.i_fall_barU_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_bar_L = obj.functions.i_fall_barL_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_z_L   = obj.functions.i_fall_zL_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
        I_z_R   = obj.functions.i_fall_zR_simp(initial_conditions.I_bar_L, initial_conditions.I_bar_U, initial_conditions.I_coil, initial_conditions.I_tw, initial_conditions.I_z_L, initial_conditions.I_z_R, initial_conditions.V_c, initial_conditions.V_s1, initial_conditions.V_s2, initial_conditions.V_s3, initial_conditions.V_s4, dt);
    end

    % Return calculated coil current
    coil_current = I_coil;
end
