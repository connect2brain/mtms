function command = create_charge_command(event_id, channel, execution_condition, time, target_voltage)
    event.event_id = uint16(event_id);
    event.execution_condition = uint8(execution_condition);
    event.time = double(time);

    command.channel = uint8(channel);
    command.event = event;
    command.waveform = get_default_waveform(channel);
    command.event_type = uint8(1);
    command.target_voltage = uint16(target_voltage);
    
    coder.cstructname(command, 'matlab_fpga_event');
    coder.cstructname(command.event, 'event');
    %coder.cstructname(command.waveform, 'waveform');
end
