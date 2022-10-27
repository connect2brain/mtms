function command = create_discharge_command(event_id, channel, execution_condition, time_us, target_voltage)
    event.event_id = uint16(event_id);
    event.execution_condition = uint8(execution_condition);
    event.time_us = uint64(time_us);
    
    command.channel = uint8(channel);
    command.event = event;
    command.pieces = get_default_waveform(channel);
    command.event_type = uint8(2);
    
    command.target_voltage = uint16(target_voltage);
    
    coder.cstructname(command, 'matlab_fpga_event');
    coder.cstructname(command.event, 'event');
    coder.cstructname(command.pieces, 'pulse_piece');
end
