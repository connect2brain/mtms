function command = create_charge_command(event_id, channel, execution_condition, execution_time, target_voltage)
    event_info.id = uint16(event_id);
    event_info.execution_condition = uint8(execution_condition);
    event_info.execution_time = double(execution_time);

    command.channel = uint8(channel);
    command.event_info = event_info;
    command.waveform = get_default_waveform(channel);
    command.event_type = uint8(1);
    command.target_voltage = uint16(target_voltage);
    command.duration_us = uint32(1000);
    command.state = uint16(0);
    
    coder.cstructname(command, 'matlab_event');
    coder.cstructname(command.event_info, 'event_info');
end
