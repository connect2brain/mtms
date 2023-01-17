function command = create_signal_out_command(event_id, port, duration_us, execution_condition, execution_time)
    event_info.id = uint16(event_id);
    event_info.execution_condition = uint8(execution_condition);
    event_info.execution_time = double(execution_time);
    
    command.channel = uint8(port);
    command.event_info = event_info;
    command.waveform = get_default_waveform(port);
    command.event_type = uint8(3);
    command.target_voltage = uint16(0);
    command.duration_us = uint32(duration_us);
    command.state = uint16(0);
    
    coder.cstructname(command, 'matlab_event');
    coder.cstructname(command.event_info, 'event_info');
end
