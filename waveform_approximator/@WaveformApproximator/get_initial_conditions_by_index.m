function ic = get_initial_conditions_by_index(obj, ic_struct, index)
    ic.V_c = ic_struct.V_c(index);
    ic.V_s1 = ic_struct.V_s1(index);
    ic.V_s2 = ic_struct.V_s2(index);
    ic.V_s3 = ic_struct.V_s3(index);
    ic.V_s4 = ic_struct.V_s4(index);
    ic.I_coil = ic_struct.I_coil(index);
    ic.I_tw = ic_struct.I_tw(index);
    ic.I_bar_U = ic_struct.I_bar_U(index);
    ic.I_bar_L = ic_struct.I_bar_L(index);
    ic.I_z_L = ic_struct.I_z_L(index);
    ic.I_z_R = ic_struct.I_z_R(index);
end