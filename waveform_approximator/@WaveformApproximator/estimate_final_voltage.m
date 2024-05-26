function final_voltage = estimate_final_voltage(obj, voltage, waveform)
    state_trajectory = obj.generate_state_trajectory_from_waveform(voltage, waveform);

    % TODO: Ensure that the final voltages computed this way match with the actual final voltages
    %   after a pulse; if not, use an alternative method to compute the final voltage.
    final_voltage = state_trajectory(end).V_c;
end
