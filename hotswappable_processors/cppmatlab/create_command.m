function command = create_command(event_id, event_type, channel, time_us, target_voltage)
    event.event_id = uint16(event_id);
    event.execution_condition = uint8(2);
    event.time_us = uint64(time_us);
    
    command.channel = uint8(channel);
    command.event = event;
    command.waveform = get_default_waveform(channel);
    command.target_voltage = uint16(target_voltage);

    if event_type == "pulse"
        command.event_type = uint8(0);
    elseif event_type == "charge"
        command.event_type = uint8(1);
    else
        command.event_type = uint8(2);
    end
    
    
    coder.cstructname(command, 'matlab_fpga_event');
    coder.cstructname(command.event, 'event');
    coder.cstructname(command.waveform, 'waveform');
end
