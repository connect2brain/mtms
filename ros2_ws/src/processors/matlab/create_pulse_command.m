function command = create_pulse_command()
    coder.inline("always");
    % Create pulse command
    event_info.event_id = uint16(0);
    event_info.wait_for_trigger = false;
    event_info.time_us = uint64(0);
    
    piece1.mode = uint8(0);
    piece1.duration_in_ticks = uint16(200);
    piece2.mode = uint8(2);
    piece2.duration_in_ticks = uint16(160);
    piece3.mode = uint8(1);
    piece3.duration_in_ticks = uint16(160);
    
    pulse_info.pieces = [piece1, piece2, piece3];
    
    pulse_command.channel = 5;
    pulse_command.event_info = event_info;
    pulse_command.pulse_info = pulse_info;
    command = pulse_command;
end