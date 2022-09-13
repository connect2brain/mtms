function command = create_command(event_type, target_voltage)
    event_info.event_id = uint16(0);
    event_info.execution_condition = uint8(0);
    event_info.time_us = uint64(0);
    
    piece1.mode = uint8(0);
    piece1.duration_in_ticks = uint16(200);
    piece2.mode = uint8(2);
    piece2.duration_in_ticks = uint16(160);
    piece3.mode = uint8(1);
    piece3.duration_in_ticks = uint16(160);
    
    pieces = [piece1, piece2, piece3];
    
    command.channel = uint8(5);
    command.event_info = event_info;
    command.pieces = pieces;
    if event_type == "pulse_event"
        command.event_type = uint8(0);
    elseif event_type == "charge_event"
        command.event_type = uint8(1);
    else
        command.event_type = uint8(2);
    end

    command.target_voltage = uint16(target_voltage);
end