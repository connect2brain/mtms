function command = create_signal_out_command(event_id, port, duration_us, execution_condition, time)
    event.id = uint16(event_id);
    event.execution_condition = uint8(execution_condition);
    event.time = double(time);
    
    command.channel = uint8(port);
    command.event = event;
    command.waveform = get_default_waveform(port);
    command.event_type = uint8(3);
    command.target_voltage = uint16(0);
    command.duration_us = uint32(duration_us);
    
    coder.cstructname(command, 'matlab_fpga_event');
    coder.cstructname(command.event, 'event');
end
