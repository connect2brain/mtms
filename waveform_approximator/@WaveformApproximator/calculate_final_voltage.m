function final_voltage = calculate_final_voltage(obj, voltage, waveform)
    timecourse = obj.calculate_timecourse(voltage, waveform);

    % TODO: Ensure that the final voltages computed this way match with the actual final voltages
    %   after a pulse; if not, use an alternative method to compute the final voltage.
    final_voltage = timecourse.V_c(end);
end
